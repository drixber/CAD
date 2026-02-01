#include "PrinterService.h"
#include "HttpClient.h"

#ifdef CAD_USE_QT
#include <QSettings>
#include <QString>
#include <QUuid>
#endif

#include <algorithm>
#include <cstdlib>
#include <ctime>

namespace cad {
namespace app {

static const char kPrintersGroup[] = "3d_printers";
static const char kPrinterCountKey[] = "count";

void PrinterService::loadPrinters() const {
    printers_.clear();
#ifdef CAD_USE_QT
    QSettings settings("HydraCAD", "HydraCAD");
    int count = settings.value(QString::fromLatin1(kPrintersGroup) + "/" + QLatin1String(kPrinterCountKey), 0).toInt();
    for (int i = 0; i < count; ++i) {
        QString prefix = QString("%1/printer_%2/").arg(kPrintersGroup).arg(i);
        PrinterProfile p;
        p.id = settings.value(prefix + "id").toString().toStdString();
        p.name = settings.value(prefix + "name").toString().toStdString();
        p.type = settings.value(prefix + "type", "octoprint").toString().toStdString();
        p.base_url = settings.value(prefix + "base_url").toString().toStdString();
        p.api_key = settings.value(prefix + "api_key").toString().toStdString();
        if (!p.id.empty()) printers_.push_back(p);
    }
#endif
}

void PrinterService::savePrinters() const {
#ifdef CAD_USE_QT
    QSettings settings("HydraCAD", "HydraCAD");
    settings.beginGroup(kPrintersGroup);
    settings.remove("");
    settings.endGroup();
    settings.setValue(QString::fromLatin1(kPrintersGroup) + "/" + QLatin1String(kPrinterCountKey), static_cast<int>(printers_.size()));
    for (size_t i = 0; i < printers_.size(); ++i) {
        QString prefix = QString("%1/printer_%2/").arg(kPrintersGroup).arg(static_cast<int>(i));
        const auto& p = printers_[i];
        settings.setValue(prefix + "id", QString::fromStdString(p.id));
        settings.setValue(prefix + "name", QString::fromStdString(p.name));
        settings.setValue(prefix + "type", QString::fromStdString(p.type));
        settings.setValue(prefix + "base_url", QString::fromStdString(p.base_url));
        settings.setValue(prefix + "api_key", QString::fromStdString(p.api_key));
    }
    settings.sync();
#endif
}

std::string PrinterService::generateId() {
#ifdef CAD_USE_QT
    return QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
#else
    std::string id;
    static const char hex[] = "0123456789abcdef";
    for (int i = 0; i < 16; ++i) {
        id += hex[std::rand() % 16];
    }
    return id;
#endif
}

std::vector<PrinterProfile> PrinterService::getPrinters() const {
    loadPrinters();
    return printers_;
}

bool PrinterService::addPrinter(const PrinterProfile& profile) {
    loadPrinters();
    PrinterProfile p = profile;
    if (p.id.empty()) p.id = generateId();
    if (p.type.empty()) p.type = "octoprint";
    printers_.push_back(p);
    savePrinters();
    return true;
}

bool PrinterService::removePrinter(const std::string& id) {
    loadPrinters();
    auto it = std::remove_if(printers_.begin(), printers_.end(),
                            [&id](const PrinterProfile& p) { return p.id == id; });
    if (it == printers_.end()) return false;
    printers_.erase(it, printers_.end());
    savePrinters();
    return true;
}

bool PrinterService::updatePrinter(const std::string& id, const PrinterProfile& profile) {
    loadPrinters();
    for (auto& p : printers_) {
        if (p.id == id) {
            p.name = profile.name;
            p.type = profile.type.empty() ? p.type : profile.type;
            p.base_url = profile.base_url;
            p.api_key = profile.api_key;
            savePrinters();
            return true;
        }
    }
    return false;
}

PrinterProfile PrinterService::getPrinter(const std::string& id) const {
    loadPrinters();
    for (const auto& p : printers_) {
        if (p.id == id) return p;
    }
    return {};
}

PrinterUploadResult PrinterService::testConnection(const std::string& printer_id) const {
    PrinterUploadResult result;
    if (!http_client_) {
        result.message = "HTTP client not available";
        return result;
    }
    PrinterProfile p = getPrinter(printer_id);
    if (p.id.empty()) {
        result.message = "Printer not found";
        return result;
    }
    std::string url;
    std::map<std::string, std::string> headers;
    if (p.type == "moonraker") {
        url = p.base_url + "/server/info";
        if (!p.api_key.empty()) headers["X-Api-Key"] = p.api_key;
    } else {
        url = p.base_url + "/api/version";
        if (!p.api_key.empty()) headers["X-Api-Key"] = p.api_key;
    }
    if (url.empty() || p.base_url.empty()) {
        result.message = "Printer URL not set";
        return result;
    }
    auto resp = http_client_->get(url, headers);
    result.status_code = resp.status_code;
    result.success = resp.success;
    result.message = resp.success ? "Connection OK" : (resp.body.empty() ? "Connection failed" : resp.body);
    return result;
}

PrinterUploadResult PrinterService::sendStlToPrinter(const std::string& printer_id,
                                                     const std::string& stl_path,
                                                     bool start_print) const {
    PrinterUploadResult result;
    if (!http_client_) {
        result.message = "HTTP client not available";
        return result;
    }
    PrinterProfile p = getPrinter(printer_id);
    if (p.id.empty()) {
        result.message = "Printer not found";
        return result;
    }
    std::string url;
    std::map<std::string, std::string> headers;
    if (p.type == "moonraker") {
        url = p.base_url + "/server/files/upload";
        if (!p.api_key.empty()) headers["X-Api-Key"] = p.api_key;
    } else {
        url = p.base_url + "/api/files/local";
        if (!p.api_key.empty()) headers["X-Api-Key"] = p.api_key;
    }
    if (url.empty()) {
        result.message = "Printer URL not set";
        return result;
    }
    HttpResponse resp = http_client_->uploadFile(url, stl_path, headers);
    result.status_code = resp.status_code;
    result.success = (resp.status_code >= 200 && resp.status_code < 300);
    result.message = resp.body.empty() ? (result.success ? "Upload OK" : "Upload failed") : resp.body;

    if (result.success && start_print && p.type == "octoprint") {
        std::string start_url = p.base_url + "/api/job";
        std::string body = "{\"command\":\"start\"}";
        headers["Content-Type"] = "application/json";
        HttpResponse start_resp = http_client_->post(start_url, body, headers);
        (void)start_resp;
    }
    return result;
}

}  // namespace app
}  // namespace cad
