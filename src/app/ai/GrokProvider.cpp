#include "GrokProvider.h"
#include "../HttpClient.h"
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <fstream>
#include <cstdio>

#ifdef CAD_USE_QT
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QString>
#endif

namespace cad {
namespace app {
namespace ai {
namespace {

bool writeTempBodyFile(const std::string& body, std::string& path_out) {
    char tmp_name[L_tmpnam];
    if (!std::tmpnam(tmp_name)) {
        return false;
    }
    path_out = tmp_name;
    std::ofstream file(path_out, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    file << body;
    return file.good();
}

}  // namespace

GrokProvider::GrokProvider() = default;

std::vector<std::string> GrokProvider::getAvailableModels() const {
    return {
        "grok-2-latest",
        "grok-2-mini"
    };
}

bool GrokProvider::setApiKey(const std::string& api_key) {
    if (api_key.empty()) {
        return false;
    }
    api_key_ = api_key;
    configured_ = true;
    return true;
}

AIResponse GrokProvider::sendRequest(const AIRequest& request) {
    AIResponse response;
    
    if (!isConfigured()) {
        response.success = false;
        response.error_message = "Grok provider not configured. Please set API key.";
        return response;
    }
    
    try {
        HttpClient http_client;
        http_client.setTimeout(60);
        
        std::string url = base_url_ + "/chat/completions";
        std::string request_body = buildRequestBody(request);
        
        std::map<std::string, std::string> headers = buildHeaders();
        headers["Content-Type"] = "application/json";
        
        HttpResponse http_response = http_client.post(url, request_body, headers);
        
        if (http_response.success && http_response.status_code == 200) {
            response = parseResponse(http_response.body);
        } else {
            response.success = false;
            response.error_message = "HTTP Error: " + std::to_string(http_response.status_code);
            if (!http_response.body.empty()) {
                response.error_message += " - " + http_response.body;
            }
        }
    } catch (const std::exception& e) {
        response.success = false;
        response.error_message = "Exception: " + std::string(e.what());
    } catch (...) {
        response.success = false;
        response.error_message = "Unknown error occurred";
    }
    
    return response;
}

bool GrokProvider::sendStreamingRequest(const AIRequest& request,
                                        std::function<void(const StreamChunk&)> callback) {
    if (!isConfigured()) {
        StreamChunk error_chunk;
        error_chunk.done = true;
        error_chunk.error = "Grok provider not configured";
        callback(error_chunk);
        return false;
    }

    AIRequest stream_request = request;
    stream_request.stream = true;
    std::string request_body = buildRequestBody(stream_request);
    std::string temp_path;
    if (!writeTempBodyFile(request_body, temp_path)) {
        StreamChunk error_chunk;
        error_chunk.done = true;
        error_chunk.error = "Failed to create temp request body";
        callback(error_chunk);
        return false;
    }

    std::map<std::string, std::string> headers = buildHeaders();
    headers["Content-Type"] = "application/json";
    headers["Accept"] = "text/event-stream";

    std::string command = "curl -s -N";
    for (const auto& header : headers) {
        command += " -H \"" + header.first + ": " + header.second + "\"";
    }
    command += " --data @\"" + temp_path + "\" \"" + (base_url_ + "/chat/completions") + "\"";

#ifdef _WIN32
    FILE* pipe = _popen(command.c_str(), "r");
#else
    FILE* pipe = popen(command.c_str(), "r");
#endif
    if (!pipe) {
        std::remove(temp_path.c_str());
        StreamChunk error_chunk;
        error_chunk.done = true;
        error_chunk.error = "curl not available for streaming";
        callback(error_chunk);
        return false;
    }

    bool received_chunk = false;
    char buffer[512];
    while (fgets(buffer, sizeof(buffer), pipe)) {
        std::string line(buffer);
        if (!line.empty() && (line.back() == '\n' || line.back() == '\r')) {
            while (!line.empty() && (line.back() == '\n' || line.back() == '\r')) {
                line.pop_back();
            }
        }
        if (line.rfind("data: ", 0) != 0) {
            continue;
        }
        StreamChunk chunk = parseStreamChunk(line);
        if (!chunk.content.empty() || chunk.done) {
            callback(chunk);
            received_chunk = true;
        }
        if (chunk.done) {
            break;
        }
    }

#ifdef _WIN32
    int rc = _pclose(pipe);
#else
    int rc = pclose(pipe);
#endif
    std::remove(temp_path.c_str());

    if (rc != 0 || !received_chunk) {
        StreamChunk error_chunk;
        error_chunk.done = true;
        error_chunk.error = "Streaming failed";
        callback(error_chunk);
        return false;
    }

    return true;
}

bool GrokProvider::testConnection() {
    if (!isConfigured()) {
        return false;
    }
    
    AIRequest test_request;
    test_request.model = "grok-2-latest";
    test_request.messages.push_back({{"user", "test"}});
    test_request.max_tokens = 5;
    
    AIResponse response = sendRequest(test_request);
    return response.success;
}

std::string GrokProvider::buildRequestBody(const AIRequest& request) const {
    #ifdef CAD_USE_QT
    QJsonObject json;
    json["model"] = QString::fromStdString(request.model);
    json["temperature"] = request.temperature;
    json["max_tokens"] = request.max_tokens;
    json["stream"] = request.stream;
    
    QJsonArray messages;
    for (const auto& msg : request.messages) {
        QJsonObject message;
        message["role"] = QString::fromStdString(msg.role);
        message["content"] = QString::fromStdString(msg.content);
        messages.append(message);
    }
    json["messages"] = messages;
    
    QJsonDocument doc(json);
    return doc.toJson(QJsonDocument::Compact).toStdString();
    #else
    std::ostringstream oss;
    oss << "{\"model\":\"" << request.model << "\",";
    oss << "\"temperature\":" << request.temperature << ",";
    oss << "\"max_tokens\":" << request.max_tokens << ",";
    oss << "\"stream\":" << (request.stream ? "true" : "false") << ",";
    oss << "\"messages\":[";
    for (size_t i = 0; i < request.messages.size(); ++i) {
        if (i > 0) oss << ",";
        oss << "{\"role\":\"" << request.messages[i].role << "\",";
        oss << "\"content\":\"" << request.messages[i].content << "\"}";
    }
    oss << "]}";
    return oss.str();
    #endif
}

AIResponse GrokProvider::parseResponse(const std::string& json_response) const {
    AIResponse response;
    
    #ifdef CAD_USE_QT
    QJsonDocument doc = QJsonDocument::fromJson(QByteArray::fromStdString(json_response));
    if (doc.isNull() || !doc.isObject()) {
        response.success = false;
        response.error_message = "Invalid JSON response";
        return response;
    }
    
    QJsonObject root = doc.object();
    
    if (root.contains("error")) {
        QJsonObject error = root["error"].toObject();
        response.success = false;
        response.error_message = error["message"].toString().toStdString();
        return response;
    }
    
    if (root.contains("choices")) {
        QJsonArray choices = root["choices"].toArray();
        if (!choices.isEmpty()) {
            QJsonObject choice = choices[0].toObject();
            QJsonObject message = choice["message"].toObject();
            response.content = message["content"].toString().toStdString();
            response.success = true;
        }
    }
    
    if (root.contains("usage")) {
        QJsonObject usage = root["usage"].toObject();
        response.tokens_used = usage["total_tokens"].toInt();
    }
    
    if (root.contains("model")) {
        response.model_used = root["model"].toString().toStdString();
    }
    #else
    size_t content_pos = json_response.find("\"content\":\"");
    if (content_pos != std::string::npos) {
        content_pos += 11;
        size_t content_end = json_response.find("\"", content_pos);
        if (content_end != std::string::npos) {
            response.content = json_response.substr(content_pos, content_end - content_pos);
            response.success = true;
        }
    }
    #endif
    
    return response;
}

StreamChunk GrokProvider::parseStreamChunk(const std::string& chunk_data) const {
    StreamChunk chunk;
    
    #ifdef CAD_USE_QT
    if (chunk_data.find("data: ") == 0) {
        std::string json_str = chunk_data.substr(6);
        if (json_str == "[DONE]") {
            chunk.done = true;
            return chunk;
        }
        
        QJsonDocument doc = QJsonDocument::fromJson(QByteArray::fromStdString(json_str));
        if (!doc.isNull() && doc.isObject()) {
            QJsonObject root = doc.object();
            if (root.contains("choices")) {
                QJsonArray choices = root["choices"].toArray();
                if (!choices.isEmpty()) {
                    QJsonObject choice = choices[0].toObject();
                    QJsonObject delta = choice["delta"].toObject();
                    if (delta.contains("content")) {
                        chunk.content = delta["content"].toString().toStdString();
                    }
                }
            }
        }
    }
    #else
    size_t content_pos = chunk_data.find("\"content\":\"");
    if (content_pos != std::string::npos) {
        content_pos += 11;
        size_t content_end = chunk_data.find("\"", content_pos);
        if (content_end != std::string::npos) {
            chunk.content = chunk_data.substr(content_pos, content_end - content_pos);
        }
    }
    #endif
    
    return chunk;
}

std::map<std::string, std::string> GrokProvider::buildHeaders() const {
    std::map<std::string, std::string> headers;
    headers["Authorization"] = "Bearer " + api_key_;
    headers["User-Agent"] = "HydraCAD/2.0.0";
    return headers;
}

}  // namespace ai
}  // namespace app
}  // namespace cad
