#pragma once

#include <string>
#include <vector>
#include <map>
#include "ai/ContextManager.h"

namespace cad {
namespace app {
namespace ai {

enum class PromptType {
    Chat,
    SketchGeneration,
    FeatureGeneration,
    DesignSuggestion,
    ErrorResolution,
    Documentation,
    ConstraintSolving
};

class PromptBuilder {
public:
    PromptBuilder();
    ~PromptBuilder() = default;
    
    // Build prompts
    std::string buildPrompt(PromptType type, const std::string& user_input, 
                           const CADContext& context = CADContext()) const;
    
    // System prompts
    std::string getSystemPrompt(PromptType type) const;
    
    // Context injection
    std::string injectContext(const std::string& prompt, const CADContext& context) const;
    
    // Template management
    void setTemplate(PromptType type, const std::string& template_str);
    std::string getTemplate(PromptType type) const;
    
private:
    std::map<PromptType, std::string> templates_;
    
    void initializeDefaultTemplates();
    std::string formatContextForPrompt(const CADContext& context) const;
};

}  // namespace ai
}  // namespace app
}  // namespace cad
