#include "AuthConfig.h"

#include <cstdlib>
#ifdef CAD_USE_QT
#include <QSettings>
#include <QString>
#endif

namespace cad {
namespace app {

AuthConfig::AuthConfig() {
    loadFromEnvironment();
#ifdef CAD_USE_QT
    loadFromSettings();
#endif
}

std::string AuthConfig::getApiBaseUrl() const {
    return api_base_url_;
}

void AuthConfig::setApiBaseUrl(const std::string& url) {
    api_base_url_ = url;
#ifdef CAD_USE_QT
    saveToSettings(url);
#endif
}

bool AuthConfig::isApiMode() const {
    return !api_base_url_.empty();
}

void AuthConfig::loadFromEnvironment() {
    const char* v = std::getenv("CAD_API_BASE_URL");
    if (v && v[0] != '\0') {
        api_base_url_ = v;
        return;
    }
    v = std::getenv("HYDRACAD_API_URL");
    if (v && v[0] != '\0') {
        api_base_url_ = v;
    }
}

#ifdef CAD_USE_QT
void AuthConfig::loadFromSettings() {
    QSettings settings("HydraCAD", "HydraCAD");
    QString url = settings.value("auth/api_base_url").toString();
    if (!url.isEmpty()) {
        api_base_url_ = url.toStdString();
    }
}

void AuthConfig::saveToSettings(const std::string& url) {
    QSettings settings("HydraCAD", "HydraCAD");
    if (url.empty()) {
        settings.remove("auth/api_base_url");
    } else {
        settings.setValue("auth/api_base_url", QString::fromStdString(url));
    }
    settings.sync();
}
#endif

}  // namespace app
}  // namespace cad
