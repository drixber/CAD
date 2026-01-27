#pragma once

#include "ai/ModelProvider.h"
#include <string>
#include <map>

namespace cad {
namespace app {
namespace ai {

class GrokProvider : public ModelProvider {
public:
    GrokProvider();
    ~GrokProvider() override = default;
    
    ModelProviderType getType() const override { return ModelProviderType::Grok; }
    std::string getName() const override { return "Grok"; }
    std::vector<std::string> getAvailableModels() const override;
    
    bool setApiKey(const std::string& api_key) override;
    bool isConfigured() const override { return configured_ && !api_key_.empty(); }
    
    AIResponse sendRequest(const AIRequest& request) override;
    bool sendStreamingRequest(const AIRequest& request,
                              std::function<void(const StreamChunk&)> callback) override;
    
    bool testConnection() override;
    
    void setBaseUrl(const std::string& url) { base_url_ = url; }
    std::string getBaseUrl() const { return base_url_; }
    
private:
    std::string base_url_{"https://api.x.ai/v1"};
    
    std::string buildRequestBody(const AIRequest& request) const;
    AIResponse parseResponse(const std::string& json_response) const;
    StreamChunk parseStreamChunk(const std::string& chunk_data) const;
    std::map<std::string, std::string> buildHeaders() const;
};

}  // namespace ai
}  // namespace app
}  // namespace cad
