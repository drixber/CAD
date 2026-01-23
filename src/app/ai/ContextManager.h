#pragma once

#include <string>
#include <vector>
#include <map>
#include "core/Modeler/Assembly.h"
#include "core/Modeler/Part.h"
#include "core/Modeler/Sketch.h"

namespace cad {
namespace app {
namespace ai {

struct SelectionContext {
    std::vector<std::string> selected_entities;
    std::string selection_type;  // "face", "edge", "vertex", "component", etc.
    std::string description;
};

struct FeatureContext {
    std::string feature_type;  // "sketch", "extrude", "revolve", etc.
    std::string feature_name;
    std::map<std::string, std::string> parameters;
    std::string description;
};

struct AssemblyContext {
    int component_count{0};
    int mate_count{0};
    std::vector<std::string> component_names;
    std::string description;
};

struct CADContext {
    SelectionContext selection;
    FeatureContext active_feature;
    AssemblyContext assembly;
    std::string workspace_mode;  // "sketch", "part", "assembly", "drawing"
    std::string current_project;
    bool has_unsaved_changes{false};
};

class ContextManager {
public:
    ContextManager();
    ~ContextManager() = default;
    
    // Context collection
    void updateSelection(const SelectionContext& selection);
    void updateActiveFeature(const FeatureContext& feature);
    void updateAssembly(const cad::core::Assembly& assembly);
    void setWorkspaceMode(const std::string& mode);
    void setCurrentProject(const std::string& project_path);
    void setUnsavedChanges(bool has_changes);
    
    // Context retrieval
    CADContext getFullContext() const;
    std::string getContextString() const;
    std::string getContextForPrompt() const;
    
    // Context clearing
    void clearSelection();
    void clearActiveFeature();
    void clearAssembly();
    void clearAll();
    
private:
    CADContext context_;
    
    std::string formatSelectionContext() const;
    std::string formatFeatureContext() const;
    std::string formatAssemblyContext() const;
};

}  // namespace ai
}  // namespace app
}  // namespace cad
