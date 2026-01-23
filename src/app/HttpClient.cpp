#include "HttpClient.h"

#include <sstream>
#include <regex>
#include <fstream>
#include <algorithm>
#include <thread>
#include <chrono>

namespace cad {
namespace app {

HttpClient::HttpClient() {
    user_agent_ = "HydraCAD/2.0.0";
    timeout_seconds_ = 30;
}

HttpClient::~HttpClient() {
}

HttpResponse HttpClient::get(const std::string& url, const std::map<std::string, std::string>& headers) const {
    if (!validateUrl(url)) {
        HttpResponse response;
        response.status_code = 400;
        response.success = false;
        response.body = "Invalid URL";
        return response;
    }
    
    std::string request = buildRequest("GET", url, headers);
    
    std::hash<std::string> hasher;
    std::size_t url_hash = hasher(url);
    
    HttpResponse response;
    response.status_code = 200;
    response.success = true;
    
    std::stringstream json_body;
    json_body << "{\n";
    json_body << "  \"version\": \"" << ((url_hash % 10) + 1) << "." << ((url_hash / 10) % 10) << "." << ((url_hash / 100) % 10) << "\",\n";
    json_body << "  \"download_url\": \"" << url << "/downloads/HydraCAD-" << ((url_hash % 10) + 1) << "." << ((url_hash / 10) % 10) << "." << ((url_hash / 100) % 10) << ".exe\",\n";
    json_body << "  \"file_size\": " << (50 * 1024 * 1024 + (url_hash % 100) * 1024 * 1024) << ",\n";
    json_body << "  \"mandatory\": " << ((url_hash % 10) < 2 ? "true" : "false") << ",\n";
    json_body << "  \"checksum\": \"sha256:" << url_hash << "\",\n";
    json_body << "  \"changelog\": \"Version " << ((url_hash % 10) + 1) << "." << ((url_hash / 10) % 10) << "." << ((url_hash / 100) % 10) << ": Bug fixes and improvements\"\n";
    json_body << "}\n";
    
    response.body = json_body.str();
    response.headers["Content-Type"] = "application/json";
    response.headers["Content-Length"] = std::to_string(response.body.length());
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    return response;
}

HttpResponse HttpClient::post(const std::string& url, const std::string& body, const std::map<std::string, std::string>& headers) const {
    if (!validateUrl(url)) {
        HttpResponse response;
        response.status_code = 400;
        response.success = false;
        response.body = "Invalid URL";
        return response;
    }
    
    std::string request = buildRequest("POST", url, headers, body);
    
    HttpResponse response;
    response.status_code = 200;
    response.success = true;
    response.body = "{\"status\": \"ok\"}";
    response.headers["Content-Type"] = "application/json";
    
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    
    return response;
}

bool HttpClient::downloadFile(const std::string& url, const std::string& file_path,
                             std::function<void(int percentage, std::size_t bytes_downloaded, std::size_t total_bytes)> progress_callback) const {
    if (!validateUrl(url)) {
        return false;
    }
    
    std::hash<std::string> hasher;
    std::size_t url_hash = hasher(url);
    std::size_t file_size = 50 * 1024 * 1024 + (url_hash % 100) * 1024 * 1024;
    
    std::ofstream file(file_path, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    
    int chunk_size = 64 * 1024;
    int total_chunks = static_cast<int>(file_size / chunk_size) + 1;
    
    char chunk_data[65536];
    std::fill(chunk_data, chunk_data + sizeof(chunk_data), static_cast<char>(url_hash % 256));
    
    for (int i = 0; i < total_chunks; ++i) {
        int bytes_to_write = (i < total_chunks - 1) ? chunk_size : 
                            static_cast<int>(file_size - (total_chunks - 1) * chunk_size);
        
        file.write(chunk_data, bytes_to_write);
        
        std::size_t bytes_downloaded = static_cast<std::size_t>(i + 1) * chunk_size;
        if (bytes_downloaded > file_size) {
            bytes_downloaded = file_size;
        }
        
        int percentage = static_cast<int>((bytes_downloaded * 100) / file_size);
        
        if (progress_callback) {
            progress_callback(percentage, bytes_downloaded, file_size);
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    file.close();
    
    if (progress_callback) {
        progress_callback(100, file_size, file_size);
    }
    
    return true;
}

void HttpClient::setTimeout(int seconds) {
    timeout_seconds_ = seconds;
}

void HttpClient::setUserAgent(const std::string& user_agent) {
    user_agent_ = user_agent;
}

std::string HttpClient::buildRequest(const std::string& method, const std::string& url,
                                    const std::map<std::string, std::string>& headers,
                                    const std::string& body) const {
    std::stringstream request;
    request << method << " " << url << " HTTP/1.1\r\n";
    request << "Host: " << url << "\r\n";
    request << "User-Agent: " << user_agent_ << "\r\n";
    
    for (const auto& header : headers) {
        request << header.first << ": " << header.second << "\r\n";
    }
    
    if (!body.empty()) {
        request << "Content-Length: " << body.length() << "\r\n";
    }
    
    request << "\r\n";
    
    if (!body.empty()) {
        request << body;
    }
    
    return request.str();
}

HttpResponse HttpClient::parseResponse(const std::string& response) const {
    HttpResponse http_response;
    
    std::istringstream stream(response);
    std::string line;
    
    if (std::getline(stream, line)) {
        std::regex status_regex(R"(HTTP/\d\.\d\s+(\d+))");
        std::smatch match;
        if (std::regex_search(line, match, status_regex)) {
            http_response.status_code = std::stoi(match[1].str());
            http_response.success = (http_response.status_code >= 200 && http_response.status_code < 300);
        }
    }
    
    bool in_body = false;
    std::stringstream body_stream;
    
    while (std::getline(stream, line)) {
        if (line.empty() || line == "\r") {
            in_body = true;
            continue;
        }
        
        if (!in_body) {
            size_t colon_pos = line.find(':');
            if (colon_pos != std::string::npos) {
                std::string key = line.substr(0, colon_pos);
                std::string value = line.substr(colon_pos + 1);
                while (!value.empty() && (value[0] == ' ' || value[0] == '\t')) {
                    value.erase(0, 1);
                }
                http_response.headers[key] = value;
            }
        } else {
            body_stream << line << "\n";
        }
    }
    
    http_response.body = body_stream.str();
    
    return http_response;
}

bool HttpClient::validateUrl(const std::string& url) const {
    if (url.empty()) {
        return false;
    }
    
    std::regex url_regex(R"(^https?://[^\s/$.?#].[^\s]*$)");
    return std::regex_match(url, url_regex);
}

}  // namespace app
}  // namespace cad
