#pragma once

#include <string>
#include <vector>
#include <functional>
#include <memory>

namespace cad {
namespace app {
namespace ai {

struct AIMessage {
    std::string role;  // "system", "user", "assistant"
    std::string content;
};

struct AIRequest {
    std::vector<AIMessage> messages;
    std::string model;
    double temperature{0.7};
    int max_tokens{2000};
    bool stream{false};
};

struct AIResponse {
    bool success{false};
    std::string content;
    std::string error_message;
    int tokens_used{0};
    std::string model_used;
};

struct StreamChunk {
    std::string content;
    bool done{false};
    std::string error;
};

enum class ModelProviderType {
    OpenAI,
    Anthropic,
    Grok,
    LocalLLM
};

class ModelProvider {
public:
    virtual ~ModelProvider() = default;
    
    // Provider identification
    virtual ModelProviderType getType() const = 0;
    virtual std::string getName() const = 0;
    virtual std::vector<std::string> getAvailableModels() const = 0;
    
    // Configuration
    virtual bool setApiKey(const std::string& api_key) = 0;
    virtual bool isConfigured() const = 0;
    
    // API calls
    virtual AIResponse sendRequest(const AIRequest& request) = 0;
    virtual bool sendStreamingRequest(const AIRequest& request,
                                      std::function<void(const StreamChunk&)> callback) = 0;
    
    // Health check
    virtual bool testConnection() = 0;
    
protected:
    std::string api_key_;
    bool configured_{false};
};

}  // namespace ai
}  // namespace app
}  // namespace cad
