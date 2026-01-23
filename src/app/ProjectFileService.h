#pragma once

#include <string>
#include <memory>
#include <vector>
#include "core/Modeler/Assembly.h"
#include "core/Modeler/Part.h"
#include "core/Modeler/Sketch.h"

namespace cad {
namespace app {

struct ProjectFileInfo {
    std::string file_path;
    std::string project_name;
    std::string version;
    std::string created_date;
    std::string modified_date;
    std::vector<std::string> assembly_ids;
    std::vector<std::string> part_ids;
};

class ProjectFileService {
public:
    ProjectFileService();
    
    // Save/Load project
    bool saveProject(const std::string& file_path, const cad::core::Assembly& assembly) const;
    bool loadProject(const std::string& file_path, cad::core::Assembly& assembly) const;
    
    // Save/Load checkpoint
    bool saveCheckpoint(const std::string& checkpoint_path, const cad::core::Assembly& assembly) const;
    bool loadCheckpoint(const std::string& checkpoint_path, cad::core::Assembly& assembly) const;
    
    // Project info
    ProjectFileInfo getProjectInfo(const std::string& file_path) const;
    bool projectExists(const std::string& file_path) const;
    
    // Auto-save
    void enableAutoSave(bool enabled);
    void setAutoSaveInterval(int seconds);
    void setAutoSavePath(const std::string& path);
    bool isAutoSaveEnabled() const;
    bool triggerAutoSave(const cad::core::Assembly& assembly) const;
    
    // Checkpoint management
    std::vector<std::string> listCheckpoints(const std::string& project_path) const;
    bool deleteCheckpoint(const std::string& checkpoint_path) const;
    void cleanupOldAutoSaves(int days_to_keep = 7) const;
    
private:
    bool auto_save_enabled_{true};
    int auto_save_interval_seconds_{300};  // 5 minutes default
    std::string auto_save_path_{"autosave"};
    
    std::string serializeAssembly(const cad::core::Assembly& assembly) const;
    bool deserializeAssembly(const std::string& data, cad::core::Assembly& assembly) const;
    std::string getTimestamp() const;
};

}  // namespace app
}  // namespace cad
