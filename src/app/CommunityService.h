#pragma once

#include <string>

namespace cad {
namespace app {

class UserAuthService;
class HttpClient;

class CommunityService {
public:
    CommunityService() = default;
    void setDependencies(UserAuthService* user_auth, HttpClient* http_client);

    /** GET /api/community/feed?sort=...&q=... → JSON string (items array). */
    std::string fetchFeed(const std::string& base_url, const std::string& sort = "new", const std::string& q = "") const;

    /** GET /api/community/items/{id}/download → save to file_path. Returns true on success. */
    bool downloadToFile(const std::string& base_url, const std::string& item_id,
                        const std::string& access_token, const std::string& file_path) const;

private:
    UserAuthService* user_auth_{nullptr};
    HttpClient* http_client_{nullptr};
};

}  // namespace app
}  // namespace cad
