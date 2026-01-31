#include "AnthropicProvider.h"
#include "../HttpClient.h"
#include <sstream>
#include <algorithm>

#ifdef CAD_USE_QT
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QString>
#endif

namespace cad {
namespace app {
namespace ai {

AnthropicProvider::AnthropicProvider() = default;

std::vector<std::string> AnthropicProvider::getAvailableModels() const {
    return {
        "claude-3-5-sonnet-20241022",
        "claude-3-5-haiku-20241022",
        "claude-3-opus-20240229",
        "claude-3-sonnet-20240229",
        "claude-3-haiku-20240307"
    };
}

bool AnthropicProvider::setApiKey(const std::string& api_key) {
    if (api_key.empty()) {
        return false;
    }
    api_key_ = api_key;
    configured_ = true;
    return true;
}

AIResponse AnthropicProvider::sendRequest(const AIRequest& request) {
    AIResponse response;

    if (!isConfigured()) {
        response.success = false;
        response.error_message = "Anthropic provider not configured. Please set API key.";
        return response;
    }

    try {
        HttpClient http_client;
        http_client.setTimeout(60);

        std::string url = base_url_ + "/messages";
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

bool AnthropicProvider::sendStreamingRequest(const AIRequest& request,
                                             std::function<void(const StreamChunk&)> callback) {
    (void)request;
    (void)callback;
    StreamChunk chunk;
    chunk.done = true;
    chunk.error = "Anthropic streaming not implemented; use non-streaming.";
    if (callback) {
        callback(chunk);
    }
    return false;
}

bool AnthropicProvider::testConnection() {
    if (!isConfigured()) {
        return false;
    }
    AIRequest test_request;
    test_request.model = "claude-3-haiku-20240307";
    test_request.messages.push_back({"user", "Say OK"});
    test_request.max_tokens = 5;
    AIResponse response = sendRequest(test_request);
    return response.success;
}

std::string AnthropicProvider::buildRequestBody(const AIRequest& request) const {
#ifdef CAD_USE_QT
    QJsonObject json;
    json["model"] = QString::fromStdString(request.model);
    json["max_tokens"] = request.max_tokens;

    std::string system_content;
    QJsonArray messages;
    for (const auto& msg : request.messages) {
        if (msg.role == "system") {
            system_content = msg.content;
            continue;
        }
        QJsonObject message;
        message["role"] = QString::fromStdString(msg.role);
        message["content"] = QString::fromStdString(msg.content);
        messages.append(message);
    }
    if (!system_content.empty()) {
        json["system"] = QString::fromStdString(system_content);
    }
    json["messages"] = messages;

    QJsonDocument doc(json);
    return doc.toJson(QJsonDocument::Compact).toStdString();
#else
    std::ostringstream oss;
    oss << "{\"model\":\"" << request.model << "\",";
    oss << "\"max_tokens\":" << request.max_tokens << ",";
    std::string system_content;
    oss << "\"messages\":[";
    bool first = true;
    for (const auto& msg : request.messages) {
        if (msg.role == "system") {
            system_content = msg.content;
            continue;
        }
        if (!first) oss << ",";
        first = false;
        oss << "{\"role\":\"" << msg.role << "\",";
        oss << "\"content\":\"" << msg.content << "\"}";
    }
    oss << "]";
    if (!system_content.empty()) {
        oss << ",\"system\":\"" << system_content << "\"";
    }
    oss << "}";
    return oss.str();
#endif
}

AIResponse AnthropicProvider::parseResponse(const std::string& json_response) const {
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

    if (root.contains("content")) {
        QJsonArray content = root["content"].toArray();
        for (const QJsonValue& val : content) {
            QJsonObject block = val.toObject();
            if (block["type"].toString() == "text") {
                response.content = block["text"].toString().toStdString();
                break;
            }
        }
        response.success = true;
    }

    if (root.contains("usage")) {
        QJsonObject usage = root["usage"].toObject();
        response.tokens_used = usage["input_tokens"].toInt(0) + usage["output_tokens"].toInt(0);
    }

    if (root.contains("model")) {
        response.model_used = root["model"].toString().toStdString();
    }
#else
    size_t text_pos = json_response.find("\"text\":\"");
    if (text_pos != std::string::npos) {
        text_pos += 8;
        size_t text_end = text_pos;
        while (text_end < json_response.size() && (json_response[text_end] != '"' || json_response[text_end - 1] == '\\')) {
            ++text_end;
        }
        if (text_end <= json_response.size()) {
            response.content = json_response.substr(text_pos, text_end - text_pos);
            response.success = true;
        }
    }
#endif

    return response;
}

StreamChunk AnthropicProvider::parseStreamChunk(const std::string& /*chunk_data*/) const {
    StreamChunk chunk;
    chunk.done = true;
    return chunk;
}

std::map<std::string, std::string> AnthropicProvider::buildHeaders() const {
    std::map<std::string, std::string> headers;
    headers["x-api-key"] = api_key_;
    headers["anthropic-version"] = "2023-06-01";
    headers["User-Agent"] = "HydraCAD/2.0.0";
    return headers;
}

}  // namespace ai
}  // namespace app
}  // namespace cad
