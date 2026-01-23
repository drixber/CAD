#include "ContextManager.h"
#include <sstream>
#include <algorithm>

namespace cad {
namespace app {
namespace ai {

ContextManager::ContextManager() {
    context_.workspace_mode = "none";
}

void ContextManager::updateSelection(const SelectionContext& selection) {
    context_.selection = selection;
}

void ContextManager::updateActiveFeature(const FeatureContext& feature) {
    context_.active_feature = feature;
}

void ContextManager::updateAssembly(const cad::core::Assembly& assembly) {
    context_.assembly.component_count = static_cast<int>(assembly.components().size());
    context_.assembly.mate_count = static_cast<int>(assembly.mates().size());
    context_.assembly.component_names.clear();
    
    for (const auto& component : assembly.components()) {
        context_.assembly.component_names.push_back(component.part.name());
    }
    
    // Build description
    std::ostringstream oss;
    oss << "Assembly with " << context_.assembly.component_count 
        << " components and " << context_.assembly.mate_count << " mates";
    context_.assembly.description = oss.str();
}

void ContextManager::setWorkspaceMode(const std::string& mode) {
    context_.workspace_mode = mode;
}

void ContextManager::setCurrentProject(const std::string& project_path) {
    context_.current_project = project_path;
}

void ContextManager::setUnsavedChanges(bool has_changes) {
    context_.has_unsaved_changes = has_changes;
}

CADContext ContextManager::getFullContext() const {
    return context_;
}

std::string ContextManager::getContextString() const {
    return getContextForPrompt();
}

std::string ContextManager::getContextForPrompt() const {
    std::ostringstream oss;
    
    oss << "CAD Context:\n";
    oss << "- Workspace Mode: " << context_.workspace_mode << "\n";
    
    if (!context_.current_project.empty()) {
        oss << "- Current Project: " << context_.current_project << "\n";
    }
    
    if (context_.has_unsaved_changes) {
        oss << "- Has Unsaved Changes: Yes\n";
    }
    
    // Selection context
    if (!context_.selection.selected_entities.empty()) {
        oss << formatSelectionContext();
    }
    
    // Feature context
    if (!context_.active_feature.feature_type.empty()) {
        oss << formatFeatureContext();
    }
    
    // Assembly context
    if (context_.assembly.component_count > 0) {
        oss << formatAssemblyContext();
    }
    
    return oss.str();
}

void ContextManager::clearSelection() {
    context_.selection = SelectionContext();
}

void ContextManager::clearActiveFeature() {
    context_.active_feature = FeatureContext();
}

void ContextManager::clearAssembly() {
    context_.assembly = AssemblyContext();
}

void ContextManager::clearAll() {
    context_ = CADContext();
}

std::string ContextManager::formatSelectionContext() const {
    std::ostringstream oss;
    oss << "\nSelection:\n";
    oss << "- Type: " << context_.selection.selection_type << "\n";
    oss << "- Entities: ";
    for (size_t i = 0; i < context_.selection.selected_entities.size(); ++i) {
        if (i > 0) oss << ", ";
        oss << context_.selection.selected_entities[i];
    }
    oss << "\n";
    if (!context_.selection.description.empty()) {
        oss << "- Description: " << context_.selection.description << "\n";
    }
    return oss.str();
}

std::string ContextManager::formatFeatureContext() const {
    std::ostringstream oss;
    oss << "\nActive Feature:\n";
    oss << "- Type: " << context_.active_feature.feature_type << "\n";
    oss << "- Name: " << context_.active_feature.feature_name << "\n";
    if (!context_.active_feature.parameters.empty()) {
        oss << "- Parameters: ";
        bool first = true;
        for (const auto& param : context_.active_feature.parameters) {
            if (!first) oss << ", ";
            oss << param.first << "=" << param.second;
            first = false;
        }
        oss << "\n";
    }
    if (!context_.active_feature.description.empty()) {
        oss << "- Description: " << context_.active_feature.description << "\n";
    }
    return oss.str();
}

std::string ContextManager::formatAssemblyContext() const {
    std::ostringstream oss;
    oss << "\nAssembly:\n";
    oss << "- Components: " << context_.assembly.component_count << "\n";
    oss << "- Mates: " << context_.assembly.mate_count << "\n";
    if (!context_.assembly.component_names.empty()) {
        oss << "- Component Names: ";
        for (size_t i = 0; i < context_.assembly.component_names.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << context_.assembly.component_names[i];
        }
        oss << "\n";
    }
    if (!context_.assembly.description.empty()) {
        oss << "- " << context_.assembly.description << "\n";
    }
    return oss.str();
}

}  // namespace ai
}  // namespace app
}  // namespace cad
