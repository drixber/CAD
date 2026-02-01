#include "CommunityService.h"
#include "UserAuthService.h"
#include "HttpClient.h"
#include <map>
#include <sstream>

namespace cad {
namespace app {

void CommunityService::setDependencies(UserAuthService* user_auth, HttpClient* http_client) {
    user_auth_ = user_auth;
    http_client_ = http_client;
}

std::string CommunityService::fetchFeed(const std::string& base_url, const std::string& sort, const std::string& q) const {
    if (base_url.empty() || !http_client_) return "{\"items\":[]}";
    std::string url = base_url + "/api/community/feed?sort=" + sort;
    if (!q.empty()) url += "&q=" + q;
    std::map<std::string, std::string> headers;
    std::string token = user_auth_ ? user_auth_->getAccessToken() : "";
    if (!token.empty()) headers["Authorization"] = "Bearer " + token;
    HttpResponse resp = http_client_->get(url, headers);
    if (resp.success && resp.status_code == 200) return resp.body;
    return "{\"items\":[]}";
}

bool CommunityService::downloadToFile(const std::string& base_url, const std::string& item_id,
                                       const std::string& access_token, const std::string& file_path) const {
    if (base_url.empty() || item_id.empty() || !http_client_) return false;
    std::string url = base_url + "/api/community/items/" + item_id + "/download";
    std::map<std::string, std::string> headers;
    if (!access_token.empty()) headers["Authorization"] = "Bearer " + access_token;
    return http_client_->downloadFile(url, file_path, nullptr);
}

}  // namespace app
}  // namespace cad
