#pragma once

#include <string>

namespace cad {
namespace app {

/**
 * Configuration for user/auth backend API (Phase 2).
 * API base URL is read from:
 * - Environment variable CAD_API_BASE_URL (or HYDRACAD_API_URL)
 * - QSettings "auth/api_base_url" when CAD_USE_QT (overrides env if set)
 * When empty, the app uses local file-based auth.
 */
class AuthConfig {
public:
    AuthConfig();

    /** Base URL for auth API (e.g. "https://api.example.com"). No trailing slash. */
    std::string getApiBaseUrl() const;
    void setApiBaseUrl(const std::string& url);

    /** Whether backend API mode is enabled (base URL is non-empty). */
    bool isApiMode() const;

private:
    std::string api_base_url_;
    void loadFromEnvironment();
#ifdef CAD_USE_QT
    void loadFromSettings();
    void saveToSettings(const std::string& url);
#endif
};

}  // namespace app
}  // namespace cad
