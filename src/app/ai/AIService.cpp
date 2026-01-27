#include "AIService.h"
#include "ai/ContextManager.h"
#include "ai/PromptBuilder.h"
#include "ai/OpenAIProvider.h"
#include "ai/GrokProvider.h"
#include <algorithm>
#include <memory>

namespace cad {
namespace app {
namespace ai {

AIService::AIService() 
    : context_manager_(std::make_unique<ContextManager>()),
      prompt_builder_(std::make_unique<PromptBuilder>()) {
    // Register default providers
    registerProvider(std::make_shared<OpenAIProvider>());
    registerProvider(std::make_shared<GrokProvider>());
    // Set OpenAI as default active provider
    if (!providers_.empty()) {
        active_provider_ = providers_[0];
    }
}

AIService::~AIService() = default;

void AIService::registerProvider(std::shared_ptr<ModelProvider> provider) {
    if (provider) {
        providers_.push_back(provider);
    }
}

void AIService::setActiveProvider(ModelProviderType type) {
    auto it = std::find_if(providers_.begin(), providers_.end(),
        [type](const std::shared_ptr<ModelProvider>& p) {
            return p->getType() == type;
        });
    if (it != providers_.end()) {
        active_provider_ = *it;
    }
}

std::shared_ptr<ModelProvider> AIService::getActiveProvider() const {
    return active_provider_;
}

std::vector<std::shared_ptr<ModelProvider>> AIService::getAvailableProviders() const {
    return providers_;
}

bool AIService::configureProvider(ModelProviderType type, const std::string& api_key) {
    auto it = std::find_if(providers_.begin(), providers_.end(),
        [type](const std::shared_ptr<ModelProvider>& p) {
            return p->getType() == type;
        });
    if (it != providers_.end()) {
        bool success = (*it)->setApiKey(api_key);
        if (success) {
            active_provider_ = *it;
        }
        return success;
    }
    return false;
}

bool AIService::isConfigured() const {
    return active_provider_ && active_provider_->isConfigured();
}

AIResponse AIService::chat(const std::string& user_message, const std::string& context) {
    if (!active_provider_ || !active_provider_->isConfigured()) {
        AIResponse response;
        response.success = false;
        response.error_message = "AI provider not configured";
        return response;
    }
    
    std::string system_prompt = buildSystemPrompt("chat");
    if (!context.empty()) {
        system_prompt += "\n\nContext: " + context;
    }
    
    AIRequest request = buildRequest(user_message, system_prompt);
    return active_provider_->sendRequest(request);
}

bool AIService::chatStreaming(const std::string& user_message,
                              std::function<void(const std::string&)> onChunk,
                              const std::string& context) {
    if (!active_provider_ || !active_provider_->isConfigured()) {
        return false;
    }
    
    std::string system_prompt = buildSystemPrompt("chat");
    if (!context.empty()) {
        system_prompt += "\n\nContext: " + context;
    }
    
    AIRequest request = buildRequest(user_message, system_prompt);
    request.stream = true;
    
    return active_provider_->sendStreamingRequest(request,
        [onChunk](const StreamChunk& chunk) {
            if (!chunk.content.empty()) {
                onChunk(chunk.content);
            }
        });
}

AIResponse AIService::generateSketch(const std::string& description) {
    if (!active_provider_ || !active_provider_->isConfigured()) {
        AIResponse response;
        response.success = false;
        response.error_message = "AI provider not configured";
        return response;
    }
    
    CADContext context = context_manager_->getFullContext();
    std::string prompt = prompt_builder_->buildPrompt(PromptType::SketchGeneration, description, context);
    
    AIRequest request = buildRequest(prompt, buildSystemPrompt("sketch_generation"));
    return active_provider_->sendRequest(request);
}

AIResponse AIService::generateFeature(const std::string& description, const std::string& sketch_context) {
    if (!active_provider_ || !active_provider_->isConfigured()) {
        AIResponse response;
        response.success = false;
        response.error_message = "AI provider not configured";
        return response;
    }
    
    CADContext context = context_manager_->getFullContext();
    std::string full_description = description;
    if (!sketch_context.empty()) {
        full_description += "\n\nSketch Context: " + sketch_context;
    }
    
    std::string prompt = prompt_builder_->buildPrompt(PromptType::FeatureGeneration, full_description, context);
    AIRequest request = buildRequest(prompt, buildSystemPrompt("feature_generation"));
    return active_provider_->sendRequest(request);
}

AIResponse AIService::suggestDesign(const std::string& current_design) {
    if (!active_provider_ || !active_provider_->isConfigured()) {
        AIResponse response;
        response.success = false;
        response.error_message = "AI provider not configured";
        return response;
    }
    
    CADContext context = context_manager_->getFullContext();
    std::string prompt = prompt_builder_->buildPrompt(PromptType::DesignSuggestion, current_design, context);
    AIRequest request = buildRequest(prompt, buildSystemPrompt("design_suggestion"));
    return active_provider_->sendRequest(request);
}

AIResponse AIService::resolveError(const std::string& error_description, const std::string& context) {
    if (!active_provider_ || !active_provider_->isConfigured()) {
        AIResponse response;
        response.success = false;
        response.error_message = "AI provider not configured";
        return response;
    }
    
    CADContext cad_context = context_manager_->getFullContext();
    std::string full_error = error_description;
    if (!context.empty()) {
        full_error += "\n\nAdditional Context: " + context;
    }
    
    std::string prompt = prompt_builder_->buildPrompt(PromptType::ErrorResolution, full_error, cad_context);
    AIRequest request = buildRequest(prompt, buildSystemPrompt("error_resolution"));
    return active_provider_->sendRequest(request);
}

AIResponse AIService::generateDocumentation(const std::string& component_description) {
    if (!active_provider_ || !active_provider_->isConfigured()) {
        AIResponse response;
        response.success = false;
        response.error_message = "AI provider not configured";
        return response;
    }
    
    CADContext context = context_manager_->getFullContext();
    std::string prompt = prompt_builder_->buildPrompt(PromptType::Documentation, component_description, context);
    AIRequest request = buildRequest(prompt, buildSystemPrompt("documentation"));
    return active_provider_->sendRequest(request);
}

AIResponse AIService::solveConstraints(const std::string& constraint_problem) {
    if (!active_provider_ || !active_provider_->isConfigured()) {
        AIResponse response;
        response.success = false;
        response.error_message = "AI provider not configured";
        return response;
    }
    
    CADContext context = context_manager_->getFullContext();
    std::string prompt = prompt_builder_->buildPrompt(PromptType::ConstraintSolving, constraint_problem, context);
    AIRequest request = buildRequest(prompt, buildSystemPrompt("constraint_solving"));
    return active_provider_->sendRequest(request);
}

void AIService::setContext(const std::string& context) {
    // Context is managed by ContextManager
    // This method can be used for additional context if needed
}

std::string AIService::getContext() const {
    return context_manager_->getContextForPrompt();
}

void AIService::clearContext() {
    context_manager_->clearAll();
}

void AIService::addToHistory(const AIMessage& message) {
    chat_history_.push_back(message);
    // Limit history size
    if (chat_history_.size() > 50) {
        chat_history_.erase(chat_history_.begin());
    }
}

std::vector<AIMessage> AIService::getHistory() const {
    return chat_history_;
}

void AIService::clearHistory() {
    chat_history_.clear();
}

void AIService::setHistory(const std::vector<AIMessage>& history) {
    chat_history_ = history;
}

void AIService::setModel(const std::string& model) {
    current_model_ = model;
}

void AIService::setTemperature(double temperature) {
    temperature_ = temperature;
}

void AIService::setMaxTokens(int max_tokens) {
    max_tokens_ = max_tokens;
}

AIRequest AIService::buildRequest(const std::string& user_message, const std::string& system_prompt) const {
    AIRequest request;
    request.model = current_model_;
    request.temperature = temperature_;
    request.max_tokens = max_tokens_;
    request.stream = false;
    
    // Build messages
    if (!system_prompt.empty()) {
        AIMessage system_msg;
        system_msg.role = "system";
        system_msg.content = system_prompt;
        request.messages.push_back(system_msg);
    }
    
    // Add history
    for (const auto& msg : chat_history_) {
        request.messages.push_back(msg);
    }
    
    // Add user message
    AIMessage user_msg;
    user_msg.role = "user";
    user_msg.content = user_message;
    request.messages.push_back(user_msg);
    
    return request;
}

std::string AIService::buildSystemPrompt(const std::string& operation_type) const {
    if (operation_type == "chat") {
        return "You are an AI assistant specialized in CAD (Computer-Aided Design) operations. "
               "You help users with CAD modeling, sketching, feature creation, assembly design, "
               "and technical documentation. Provide clear, accurate, and helpful responses.";
    } else if (operation_type == "sketch_generation") {
        return "You are a CAD sketch generation assistant. Generate CAD sketch commands and descriptions "
               "based on user requirements. Provide clear, executable CAD operations.";
    } else if (operation_type == "feature_generation") {
        return "You are a CAD feature generation assistant. Generate CAD feature operations (extrude, "
               "revolve, fillet, etc.) based on user requirements and sketch context.";
    } else if (operation_type == "design_suggestion") {
        return "You are a CAD design consultant. Analyze designs and suggest improvements for "
               "functionality, manufacturability, weight reduction, and best practices.";
    } else if (operation_type == "error_resolution") {
        return "You are a CAD troubleshooting assistant. Analyze errors and provide solutions "
               "for constraint conflicts, modeling issues, and design problems.";
    } else if (operation_type == "documentation") {
        return "You are a technical documentation assistant. Generate clear, comprehensive "
               "technical documentation for CAD components, assemblies, and designs.";
    } else if (operation_type == "constraint_solving") {
        return "You are a CAD constraint solver assistant. Help resolve constraint conflicts "
               "and suggest constraint modifications for properly constrained sketches.";
    }
    
    return "You are a CAD assistant. Help users with CAD operations.";
}

}  // namespace ai
}  // namespace app
}  // namespace cad
