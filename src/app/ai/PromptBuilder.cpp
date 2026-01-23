#include "PromptBuilder.h"
#include <sstream>

namespace cad {
namespace app {
namespace ai {

PromptBuilder::PromptBuilder() {
    initializeDefaultTemplates();
}

std::string PromptBuilder::buildPrompt(PromptType type, const std::string& user_input, 
                                       const CADContext& context) const {
    std::string prompt = getSystemPrompt(type);
    
    if (!context.workspace_mode.empty() || 
        !context.selection.selected_entities.empty() ||
        !context.active_feature.feature_type.empty() ||
        context.assembly.component_count > 0) {
        prompt += "\n\n" + formatContextForPrompt(context);
    }
    
    prompt += "\n\nUser Request: " + user_input;
    
    return prompt;
}

std::string PromptBuilder::getSystemPrompt(PromptType type) const {
    if (templates_.find(type) != templates_.end()) {
        return templates_.at(type);
    }
    
    // Fallback to default
    return "You are a CAD assistant. Help users with CAD operations.";
}

std::string PromptBuilder::injectContext(const std::string& prompt, const CADContext& context) const {
    std::string result = prompt;
    std::string context_str = formatContextForPrompt(context);
    
    // Inject context at the beginning
    if (!context_str.empty()) {
        result = context_str + "\n\n" + result;
    }
    
    return result;
}

void PromptBuilder::setTemplate(PromptType type, const std::string& template_str) {
    templates_[type] = template_str;
}

std::string PromptBuilder::getTemplate(PromptType type) const {
    if (templates_.find(type) != templates_.end()) {
        return templates_.at(type);
    }
    return "";
}

void PromptBuilder::initializeDefaultTemplates() {
    // Chat template
    templates_[PromptType::Chat] = 
        "You are an AI assistant specialized in CAD (Computer-Aided Design) operations. "
        "You help users with CAD modeling, sketching, feature creation, assembly design, "
        "and technical documentation. Provide clear, accurate, and helpful responses. "
        "When suggesting CAD operations, use clear, executable commands.";
    
    // Sketch Generation template
    templates_[PromptType::SketchGeneration] = 
        "You are a CAD sketch generation assistant. Generate CAD sketch commands and descriptions "
        "based on user requirements. Provide clear, executable CAD operations.\n\n"
        "Common sketch operations:\n"
        "- Line: Create a line between two points\n"
        "- Rectangle: Create a rectangle with width and height\n"
        "- Circle: Create a circle with center and radius\n"
        "- Arc: Create an arc with center, start, and end points\n"
        "- Constraints: Add geometric constraints (coincident, parallel, perpendicular, etc.)\n\n"
        "Format your response as clear CAD commands that can be executed.";
    
    // Feature Generation template
    templates_[PromptType::FeatureGeneration] = 
        "You are a CAD feature generation assistant. Generate CAD feature operations based on "
        "user requirements and sketch context.\n\n"
        "Common feature operations:\n"
        "- Extrude: Extrude a sketch profile to create a 3D feature\n"
        "- Revolve: Revolve a sketch profile around an axis\n"
        "- Loft: Create a loft between multiple profiles\n"
        "- Fillet: Add rounded edges with specified radius\n"
        "- Chamfer: Add chamfered edges\n"
        "- Hole: Create holes with specified diameter and depth\n\n"
        "Consider the active sketch and provide appropriate feature parameters.";
    
    // Design Suggestion template
    templates_[PromptType::DesignSuggestion] = 
        "You are a CAD design consultant. Analyze designs and suggest improvements for:\n"
        "- Functionality and performance\n"
        "- Manufacturability and ease of production\n"
        "- Weight reduction and material optimization\n"
        "- Best practices and industry standards\n"
        "- Cost reduction opportunities\n\n"
        "Provide specific, actionable suggestions with reasoning.";
    
    // Error Resolution template
    templates_[PromptType::ErrorResolution] = 
        "You are a CAD troubleshooting assistant. Analyze errors and provide solutions for:\n"
        "- Constraint conflicts and over-constrained sketches\n"
        "- Modeling issues and geometry problems\n"
        "- Assembly mate conflicts\n"
        "- Feature creation failures\n"
        "- Import/export errors\n\n"
        "Provide step-by-step solutions to resolve the issues.";
    
    // Documentation template
    templates_[PromptType::Documentation] = 
        "You are a technical documentation assistant. Generate clear, comprehensive "
        "technical documentation for CAD components, assemblies, and designs.\n\n"
        "Include:\n"
        "- Component descriptions and specifications\n"
        "- Assembly instructions and relationships\n"
        "- Material and manufacturing notes\n"
        "- Dimensions and tolerances\n"
        "- Bill of Materials (BOM) when applicable\n\n"
        "Format the documentation in a clear, professional manner.";
    
    // Constraint Solving template
    templates_[PromptType::ConstraintSolving] = 
        "You are a CAD constraint solver assistant. Help resolve constraint conflicts "
        "and suggest constraint modifications for properly constrained sketches.\n\n"
        "Common constraint issues:\n"
        "- Over-constrained: Too many constraints, remove redundant ones\n"
        "- Under-constrained: Not enough constraints, add missing constraints\n"
        "- Conflicting constraints: Constraints that contradict each other\n\n"
        "Analyze the constraint problem and suggest specific solutions.";
}

std::string PromptBuilder::formatContextForPrompt(const CADContext& context) const {
    std::ostringstream oss;
    
    oss << "Current CAD Context:\n";
    oss << "Workspace: " << context.workspace_mode << "\n";
    
    if (!context.current_project.empty()) {
        oss << "Project: " << context.current_project << "\n";
    }
    
    if (context.has_unsaved_changes) {
        oss << "Status: Has unsaved changes\n";
    }
    
    // Selection
    if (!context.selection.selected_entities.empty()) {
        oss << "\nSelected: ";
        for (size_t i = 0; i < context.selection.selected_entities.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << context.selection.selected_entities[i];
        }
        oss << " (" << context.selection.selection_type << ")\n";
    }
    
    // Active Feature
    if (!context.active_feature.feature_type.empty()) {
        oss << "\nActive Feature: " << context.active_feature.feature_type;
        if (!context.active_feature.feature_name.empty()) {
            oss << " (" << context.active_feature.feature_name << ")";
        }
        oss << "\n";
    }
    
    // Assembly
    if (context.assembly.component_count > 0) {
        oss << "\nAssembly: " << context.assembly.component_count 
            << " components, " << context.assembly.mate_count << " mates\n";
    }
    
    return oss.str();
}

}  // namespace ai
}  // namespace app
}  // namespace cad
