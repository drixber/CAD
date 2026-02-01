#pragma once

#include <string>
#include <ctime>

namespace cad {
namespace app {

class UserAuthService;
class HttpClient;

/**
 * License check/activate against backend API.
 * Caches result (e.g. 24h or until expires_at); isAllowed(feature) for gating.
 */
class LicenseService {
public:
    LicenseService();
    void setUserAuthService(UserAuthService* auth);
    void setHttpClient(HttpClient* client);

    /** Machine ID for this installation (persisted). */
    std::string getMachineId() const;

    /** Call POST /api/license/check; cache result. Returns true if valid. */
    bool check();
    /** Call POST /api/license/activate with key; update cache. Returns true if success. */
    bool activate(const std::string& license_key);
    /** Whether a feature is allowed by current license (free: basic; trial/pro/enterprise: more). */
    bool isAllowed(const std::string& feature) const;

    std::string getLicenseType() const { return cached_license_type_; }
    std::string getExpiresAt() const { return cached_expires_at_; }
    std::string getLastError() const { return last_error_; }

private:
    UserAuthService* user_auth_{nullptr};
    HttpClient* http_client_{nullptr};
    std::string cached_license_type_;
    std::string cached_expires_at_;
    bool cached_valid_{false};
    std::time_t cache_until_{0};
    std::string last_error_;
    static const int CACHE_SECONDS = 86400;  // 24h

    bool ensureMachineId();
    std::string machine_id_;
};

}  // namespace app
}  // namespace cad
