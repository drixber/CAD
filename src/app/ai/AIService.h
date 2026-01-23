#pragma once

#include <string>
#include <vector>
#include <functional>
#include <memory>
#include "ai/ModelProvider.h"

namespace cad {
namespace app {
namespace ai {

class ContextManager;
class PromptBuilder;

class AIService {
public:
    AIService();
    ~AIService();
    
    // Provider management
    void registerProvider(std::shared_ptr<ModelProvider> provider);
    void setActiveProvider(ModelProviderType type);
    std::shared_ptr<ModelProvider> getActiveProvider() const;
    std::vector<std::shared_ptr<ModelProvider>> getAvailableProviders() const;
    
    // Configuration
    bool configureProvider(ModelProviderType type, const std::string& api_key);
    bool isConfigured() const;
    
    // Chat operations
    AIResponse chat(const std::string& user_message, const std::string& context = "");
    bool chatStreaming(const std::string& user_message,
                      std::function<void(const std::string&)> onChunk,
                      const std::string& context = "");
    
    // CAD-specific operations
    AIResponse generateSketch(const std::string& description);
    AIResponse generateFeature(const std::string& description, const std::string& sketch_context = "");
    AIResponse suggestDesign(const std::string& current_design);
    AIResponse resolveError(const std::string& error_description, const std::string& context = "");
    AIResponse generateDocumentation(const std::string& component_description);
    AIResponse solveConstraints(const std::string& constraint_problem);
    
    // Context management
    void setContext(const std::string& context);
    std::string getContext() const;
    void clearContext();
    
    // History management
    void addToHistory(const AIMessage& message);
    std::vector<AIMessage> getHistory() const;
    void clearHistory();
    void setHistory(const std::vector<AIMessage>& history);
    
    // Settings
    void setModel(const std::string& model);
    void setTemperature(double temperature);
    void setMaxTokens(int max_tokens);
    
private:
    std::vector<std::shared_ptr<ModelProvider>> providers_;
    std::shared_ptr<ModelProvider> active_provider_;
    std::unique_ptr<ContextManager> context_manager_;
    std::unique_ptr<PromptBuilder> prompt_builder_;
    std::vector<AIMessage> chat_history_;
    
    std::string current_model_{"gpt-4"};
    double temperature_{0.7};
    int max_tokens_{2000};
    
    AIRequest buildRequest(const std::string& user_message, const std::string& system_prompt = "") const;
    std::string buildSystemPrompt(const std::string& operation_type) const;
};

}  // namespace ai
}  // namespace app
}  // namespace cad
