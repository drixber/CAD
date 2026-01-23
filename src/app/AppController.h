#pragma once

#include "CADApplication.h"
#include "core/Modeler/Modeler.h"
#include "core/FreeCAD/FreeCADAdapter.h"
#include "core/FreeCAD/TechDrawBridge.h"
#include "core/analysis/InterferenceChecker.h"
#include "core/assembly/AssemblyManager.h"
#include "core/perf/PerfSpan.h"
#include "ui/MainWindow.h"
#include "modules/drawings/DrawingService.h"
#include "modules/drawings/BomService.h"
#include "modules/drawings/AnnotationService.h"
#include "modules/drawings/AssociativeLinkService.h"
#include "modules/sheetmetal/SheetMetalService.h"
#include "modules/simulation/SimulationService.h"
#include "modules/patterns/PatternService.h"
#include "modules/direct/DirectEditService.h"
#include "modules/routing/RoutingService.h"
#include "modules/visualization/VisualizationService.h"
#include "modules/mbd/MbdService.h"
#include "modules/simplify/SimplifyService.h"
#include "interop/ImportExportService.h"
#include "interop/IoPipeline.h"
#include "UpdateService.h"
#include "ProjectFileService.h"
#include "UserAuthService.h"
#include "ai/AIService.h"
#include <vector>
#include <filesystem>
#include <algorithm>

namespace cad {
namespace app {

class AppController {
public:
    AppController();

    void initialize();
    bool initializeWithLogin();
    void setActiveSketch(const cad::core::Sketch& sketch);
    cad::ui::MainWindow& mainWindow();
    cad::core::Modeler& modeler();
    cad::core::FreeCADAdapter& freecad();
    
    // User management
    bool isUserLoggedIn() const;
    cad::app::User getCurrentUser() const;
    void logout();
    
    // Project file operations
    bool saveProject(const std::string& file_path);
    bool loadProject(const std::string& file_path);
    bool saveCheckpoint(const std::string& checkpoint_path);
    bool loadCheckpoint(const std::string& checkpoint_path);
    void triggerAutoSave();
    std::vector<std::string> getRecentProjects() const;
    bool hasUnsavedChanges() const { return has_unsaved_changes_; }
    std::string getCurrentProjectPath() const { return current_project_path_; }

private:
    std::string buildParameterSummary(const cad::core::Sketch& sketch) const;
    void initializeAssembly();
    void bindCommands();
    void executeCommand(const std::string& command);

    cad::ui::MainWindow main_window_;
    cad::core::Modeler modeler_;
    cad::core::FreeCADAdapter freecad_;
    cad::core::TechDrawBridge techdraw_bridge_;
    cad::core::InterferenceChecker interference_checker_;
    cad::core::AssemblyManager assembly_manager_;
    cad::modules::DrawingService drawing_service_;
    cad::drawings::BomService bom_service_;
    cad::drawings::AnnotationService annotation_service_;
    cad::drawings::AssociativeLinkService associative_link_service_;
    cad::modules::SheetMetalService sheet_metal_service_;
    cad::modules::SimulationService simulation_service_;
    cad::modules::PatternService pattern_service_;
    cad::modules::DirectEditService direct_edit_service_;
    cad::modules::RoutingService routing_service_;
    cad::modules::VisualizationService visualization_service_;
    cad::modules::MbdService mbd_service_;
    cad::modules::SimplifyService simplify_service_;
    cad::interop::ImportExportService io_service_;
    cad::interop::IoPipeline io_pipeline_;
    cad::app::UpdateService update_service_;
    cad::app::UpdateInstaller update_installer_;
    cad::app::ProjectFileService project_file_service_;
    cad::app::UserAuthService user_auth_service_;
    cad::app::ai::AIService ai_service_;
    cad::core::Sketch active_sketch_{"Sketch"};
    cad::core::Assembly active_assembly_;
    std::vector<std::string> recent_projects_;
    bool has_unsaved_changes_{false};
    std::string current_project_path_;
    
    void saveRecentProjectsToSettings() const;
    void markProjectModified();
    void setupAIService(cad::ui::QtMainWindow* qt_window);
    void showAISettingsDialog(cad::ui::QtMainWindow* qt_window);
    void setupAutoUpdate(cad::ui::QtMainWindow* qt_window);
    void checkForUpdates(cad::ui::QtMainWindow* qt_window, bool auto_install = false);
    void installUpdate(cad::ui::QtMainWindow* qt_window, 
                      const cad::app::UpdateInfo& update_info, 
                      bool create_backup = true);

}  // namespace app
}  // namespace cad
