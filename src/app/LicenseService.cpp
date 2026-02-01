#include "LicenseService.h"
#include "UserAuthService.h"
#include "HttpClient.h"

#include <sstream>
#include <iomanip>
#include <random>
#include <algorithm>
#ifdef CAD_USE_QT
#include <QSettings>
#include <QStandardPaths>
#include <QCryptographicHash>
#include <QByteArrayView>
#endif

namespace cad {
namespace app {

LicenseService::LicenseService() = default;

void LicenseService::setUserAuthService(UserAuthService* auth) {
    user_auth_ = auth;
}

void LicenseService::setHttpClient(HttpClient* client) {
    http_client_ = client;
}

static std::string extractJsonString(const std::string& body, const std::string& key) {
    std::string needle = "\"" + key + "\":\"";
    size_t pos = body.find(needle);
    if (pos == std::string::npos) {
        needle = "\"" + key + "\":";
        pos = body.find(needle);
        if (pos == std::string::npos) return {};
        pos += needle.size();
        while (pos < body.size() && (body[pos] == ' ' || body[pos] == '\t')) ++pos;
        if (pos < body.size() && (body[pos] == 't' || body[pos] == 'f')) {
            if (body.substr(pos, 4) == "true") return "true";
            if (body.substr(pos, 5) == "false") return "false";
        }
        return {};
    }
    pos += needle.size();
    size_t end = pos;
    while (end < body.size() && (body[end] != '"' || (end > pos && body[end - 1] == '\\'))) ++end;
    if (end > body.size()) return {};
    return body.substr(pos, end - pos);
}

bool LicenseService::ensureMachineId() {
    if (!machine_id_.empty()) return true;
#ifdef CAD_USE_QT
    QSettings settings("HydraCAD", "HydraCAD");
    QString stored = settings.value("license/machine_id").toString();
    if (!stored.isEmpty()) {
        machine_id_ = stored.toStdString();
        return true;
    }
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (path.isEmpty()) path = "hydracad";
    QByteArray hash = QCryptographicHash::hash(
        QByteArrayView(path.toUtf8()),
        QCryptographicHash::Sha256
    );
    machine_id_ = hash.toHex().toStdString();
    if (machine_id_.size() > 32) machine_id_ = machine_id_.substr(0, 32);
    settings.setValue("license/machine_id", QString::fromStdString(machine_id_));
    settings.sync();
    return true;
#else
    std::string seed = "hydracad";
    std::mt19937 gen(std::hash<std::string>{}(seed));
    std::uniform_int_distribution<> dis(0, 15);
    const char hex[] = "0123456789abcdef";
    for (int i = 0; i < 32; ++i) machine_id_ += hex[dis(gen)];
    return true;
#endif
}

std::string LicenseService::getMachineId() const {
    const_cast<LicenseService*>(this)->ensureMachineId();
    return machine_id_;
}

bool LicenseService::check() {
    if (!user_auth_ || !http_client_ || !user_auth_->isLoggedIn()) {
        cached_valid_ = false;
        last_error_ = "Not logged in";
        return false;
    }
    if (user_auth_->getApiBaseUrl().empty()) {
        cached_valid_ = true;
        cached_license_type_ = "free";
        return true;
    }
    std::string token = user_auth_->getAccessToken();
    if (token.empty()) {
        cached_valid_ = false;
        last_error_ = "No access token";
        return false;
    }
    if (std::time(nullptr) < cache_until_ && cached_valid_) {
        return true;
    }
    ensureMachineId();
    std::string base = user_auth_->getApiBaseUrl();
    if (base.empty()) {
        cached_valid_ = true;
        cached_license_type_ = "free";
        return true;
    }
    std::string url = base + "/api/license/check";
    std::string body = "{\"machine_id\":\"" + machine_id_ + "\"}";
    std::map<std::string, std::string> headers = {
        {"Authorization", "Bearer " + token},
        {"Content-Type", "application/json"}
    };
    HttpResponse resp = http_client_->post(url, body, headers);
    if (resp.status_code == 401) {
        cached_valid_ = false;
        last_error_ = "Session expired";
        return false;
    }
    if (!resp.success || resp.status_code != 200) {
        cached_valid_ = false;
        last_error_ = "License check failed";
        return false;
    }
    std::string valid_s = extractJsonString(resp.body, "valid");
    cached_valid_ = (valid_s == "true");
    cached_license_type_ = extractJsonString(resp.body, "license_type");
    cached_expires_at_ = extractJsonString(resp.body, "expires_at");
    last_error_ = extractJsonString(resp.body, "error");
    cache_until_ = std::time(nullptr) + CACHE_SECONDS;
    return cached_valid_;
}

bool LicenseService::activate(const std::string& license_key) {
    if (!user_auth_ || !http_client_ || !user_auth_->isLoggedIn()) {
        last_error_ = "Not logged in";
        return false;
    }
    std::string token = user_auth_->getAccessToken();
    if (token.empty()) {
        last_error_ = "No access token";
        return false;
    }
    ensureMachineId();
    std::string base = user_auth_->getApiBaseUrl();
    if (base.empty()) {
        last_error_ = "API URL not set";
        return false;
    }
    std::string url = base + "/api/license/activate";
    std::string body = "{\"license_key\":\"" + license_key + "\",\"machine_id\":\"" + machine_id_ + "\"}";
    std::map<std::string, std::string> headers = {
        {"Authorization", "Bearer " + token},
        {"Content-Type", "application/json"}
    };
    HttpResponse resp = http_client_->post(url, body, headers);
    if (!resp.success || resp.status_code != 200) {
        last_error_ = extractJsonString(resp.body, "error").empty() ? "Activation failed" : extractJsonString(resp.body, "error");
        return false;
    }
    std::string valid_s = extractJsonString(resp.body, "valid");
    cached_valid_ = (valid_s == "true");
    cached_license_type_ = extractJsonString(resp.body, "license_type");
    cached_expires_at_ = extractJsonString(resp.body, "expires_at");
    cache_until_ = std::time(nullptr) + CACHE_SECONDS;
    return cached_valid_;
}

bool LicenseService::isAllowed(const std::string& feature) const {
    if (cached_license_type_.empty()) return true;  // no API or not checked yet
    if (cached_license_type_ == "enterprise") return true;
    if (cached_license_type_ == "pro") {
        if (feature == "export" || feature == "cloud" || feature == "pro_module") return true;
        return true;
    }
    if (cached_license_type_ == "trial") {
        if (feature == "export") return true;
        return true;
    }
    return true;  // free: allow basic
}

}  // namespace app
}  // namespace cad
