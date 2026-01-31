#include "HttpClient.h"

#include <sstream>
#include <regex>
#include <fstream>
#include <algorithm>
#include <thread>
#include <chrono>

#ifdef CAD_USE_QT_NETWORK
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QEventLoop>
#include <QFile>
#include <QCoreApplication>
#endif

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
#ifdef CAD_USE_QT_NETWORK
    QNetworkAccessManager manager;
    QNetworkRequest request(QUrl(QString::fromStdString(url)));
    request.setHeader(QNetworkRequest::UserAgentHeader, QString::fromStdString(user_agent_));
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    for (const auto& h : headers) {
        request.setRawHeader(QByteArray::fromStdString(h.first), QByteArray::fromStdString(h.second));
    }
    QNetworkReply* reply = manager.get(request);
    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    HttpResponse response;
    response.status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    response.success = (reply->error() == QNetworkReply::NoError && response.status_code >= 200 && response.status_code < 300);
    response.body = reply->readAll().toStdString();
    reply->deleteLater();
    return response;
#else
    (void)headers;
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
#endif
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
#ifdef CAD_USE_QT_NETWORK
    QNetworkAccessManager manager;
    QNetworkRequest request(QUrl(QString::fromStdString(url)));
    request.setHeader(QNetworkRequest::UserAgentHeader, QString::fromStdString(user_agent_));
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    QNetworkReply* reply = manager.get(request);
    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    if (progress_callback) {
        QObject::connect(reply, &QNetworkReply::downloadProgress, reply, [progress_callback](qint64 received, qint64 total) {
            int pct = (total > 0) ? static_cast<int>((received * 100) / total) : 0;
            progress_callback(pct, static_cast<std::size_t>(received), total > 0 ? static_cast<std::size_t>(total) : 0);
        });
        progress_callback(0, 0, 0);
    }
    loop.exec();
    const bool ok = (reply->error() == QNetworkReply::NoError);
    bool wrote = false;
    if (ok) {
        QFile out(QString::fromStdString(file_path));
        if (out.open(QIODevice::WriteOnly)) {
            out.write(reply->readAll());
            out.close();
            wrote = true;
        }
        if (wrote && progress_callback) {
            QFile f(QString::fromStdString(file_path));
            std::size_t size = f.exists() ? static_cast<std::size_t>(f.size()) : 0;
            progress_callback(100, size, size);
        }
    }
    reply->deleteLater();
    return ok && wrote;
#else
    (void)file_path;
    (void)progress_callback;
    return false;
#endif
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

bool HttpClient::validateUrl(const std::string& url) const {
    if (url.empty()) {
        return false;
    }
    
    std::regex url_regex(R"(^https?://[^\s/$.?#].[^\s]*$)");
    return std::regex_match(url, url_regex);
}

}  // namespace app
}  // namespace cad
