#pragma once

#include <string>
#include <map>
#include <vector>
#include <functional>

namespace cad {
namespace app {

struct HttpResponse {
    int status_code{0};
    std::string body;
    std::map<std::string, std::string> headers;
    bool success{false};
};

class HttpClient {
public:
    HttpClient();
    ~HttpClient();
    
    HttpResponse get(const std::string& url, const std::map<std::string, std::string>& headers = {}) const;
    HttpResponse post(const std::string& url, const std::string& body, const std::map<std::string, std::string>& headers = {}) const;
    bool downloadFile(const std::string& url, const std::string& file_path,
                     std::function<void(int percentage, std::size_t bytes_downloaded, std::size_t total_bytes)> progress_callback = nullptr) const;
    
    void setTimeout(int seconds);
    void setUserAgent(const std::string& user_agent);
    
private:
    int timeout_seconds_{30};
    std::string user_agent_;
    
    std::string buildRequest(const std::string& method, const std::string& url,
                            const std::map<std::string, std::string>& headers,
                            const std::string& body = "") const;
    HttpResponse parseResponse(const std::string& response) const;
    bool validateUrl(const std::string& url) const;
};

}  // namespace app
}  // namespace cad
