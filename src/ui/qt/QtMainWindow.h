#pragma once

#include <QMainWindow>
#include <functional>
#include <vector>
#include <QLabel>

#include "QtBrowserTree.h"
#include "QtCommandLine.h"
#include "QtPropertyPanel.h"
#include "QtRibbon.h"
#include "QtAgentConsole.h"
#include "QtAgentThoughts.h"
#include "QtViewport.h"
#include "QtLogPanel.h"
#include "QtPerformancePanel.h"
#include "QtCommunityPanel.h"
#include "QtAIChatPanel.h"
#include "viewport/Viewport3D.h"
#include "core/Modeler/Sketch.h"
#include <QTimer>
#include <QMenuBar>
#include <QFileDialog>
#include <QCloseEvent>

namespace cad {
namespace ui {

class QtMainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit QtMainWindow(QWidget* parent = nullptr);
    ~QtMainWindow() override;
    void updateFromSketch(const cad::core::Sketch& sketch);
    void setConstraintCount(int count);
    void setParameterCount(int count);
    void setParameterSummary(const std::string& summary);
    void setParameterTable(const std::vector<std::string>& names, const std::vector<double>& values, const std::vector<std::string>& expressions);
    void setIntegrationStatus(const std::string& status);
    void setMateCount(int count);
    void setCommandHandler(const std::function<void(const std::string&)>& handler);
    void setAssemblySummary(const std::string& summary);
    void setMatesSummary(const std::string& summary);
    void setReferenceGeometry(const QStringList& planeNames, const QStringList& axisNames, const QStringList& pointNames);
    void setContextPlaceholder(const std::string& context);
    void appendRecentCommand(const std::string& command);
    void setContextCategory(const std::string& category);
    void setViewportStatus(const std::string& status);
    void setWorkspaceMode(const std::string& mode);
    void setDocumentLabel(const std::string& label);
    void setCacheStats(int entries, int max_entries);
    void setBackgroundLoading(bool enabled);
    void setLodModeHandler(const std::function<void(const std::string&)>& handler);
    void setBackgroundLoadingHandler(const std::function<void(bool)>& handler);
    void setLoadProgress(int progress);
    void setTargetFps(int fps);
    void setTargetFpsHandler(const std::function<void(int)>& handler);
    
    QtPropertyPanel* propertyPanel();
    QtCommandLine* commandLine();
    Viewport3D* viewport3D();
    QtAIChatPanel* aiChatPanel() { return ai_chat_panel_; }
    QtCommunityPanel* communityPanel() { return community_panel_; }
    void setCurrentUser(const std::string& username, const std::string& email);
    void setLogoutHandler(const std::function<void()>& handler);
    void setProfileHandler(const std::function<void()>& handler);

    // Project file operations
    void setNewProjectHandler(const std::function<void()>& handler);
    void setNewProjectFromTemplateHandler(const std::function<void(const std::string&)>& handler);
    void setTemplateDirectory(const QString& path);
    void setSaveProjectHandler(const std::function<void(const std::string&)>& handler);
    void setLoadProjectHandler(const std::function<void(const std::string&)>& handler);
    void setCurrentProjectPath(const QString& path);
    QString currentProjectPath() const;
    void setAutoSaveTriggerHandler(const std::function<void()>& handler);
    void setAutoSaveStatusHandler(const std::function<void(const std::string&)>& handler);
    void setCheckForUpdatesHandler(const std::function<void()>& handler);
    void setInstallUpdateFromFileHandler(const std::function<void()>& handler);
    void setLicenseActivateHandler(const std::function<void()>& handler);
    void setPrintersDialogHandler(const std::function<void()>& handler);
    void setSendToPrinterHandler(const std::function<void()>& handler);
    void updateRecentProjectsMenu(const std::vector<std::string>& projects);
    void setCheckpointsListProvider(const std::function<std::vector<std::string>(const std::string&)>& provider);
    void setLoadCheckpointHandler(const std::function<void(const std::string&)>& handler);
    void setDeleteCheckpointHandler(const std::function<void(const std::string&)>& handler);
    void showCheckpointsDialog();

    void setPropertyApplyHandler(const std::function<void()>& handler);
    void setPropertyCancelHandler(const std::function<void()>& handler);
    void setPropertyApplyAndNewHandler(const std::function<void()>& handler);

    void setImportFileHandler(const std::function<void(const std::string& path, const std::string& format)>& handler);
    void setExportFileHandler(const std::function<void(const std::string& path, const std::string& format)>& handler);
    void triggerImportDialog();
    void triggerExportDialog();
    void triggerNewProject();
    void triggerOpenProject();
    void triggerSaveProject();

    void executeCommand(const std::string& command);

    enum class UnsavedAction { Cancel = 0, Save = 1, Discard = 2 };
    void setAskUnsavedChangesHandler(const std::function<UnsavedAction()>& handler);
    void setGetSavePathHandler(const std::function<std::string()>& handler);
    UnsavedAction askUnsavedChanges() const;
    std::string getSavePathForNewProject() const;

private:
    QtRibbon* ribbon_{nullptr};
    QtBrowserTree* browser_tree_{nullptr};
    QtPropertyPanel* property_panel_{nullptr};
    QtCommandLine* command_line_{nullptr};
    QtAgentConsole* agent_console_{nullptr};
    QtAgentThoughts* agent_thoughts_{nullptr};
    QtViewport* viewport_{nullptr};
    QtLogPanel* log_panel_{nullptr};
    QtPerformancePanel* perf_panel_{nullptr};
    QtAIChatPanel* ai_chat_panel_{nullptr};
    QtCommunityPanel* community_panel_{nullptr};
    QLabel* mode_label_{nullptr};
    QLabel* document_label_{nullptr};
    QLabel* fps_status_label_{nullptr};
    QLabel* autosave_status_label_{nullptr};
    QTimer* autosave_timer_{nullptr};
    QMenu* recent_projects_menu_{nullptr};
    QLabel* user_label_{nullptr};
    QMenu* user_menu_{nullptr};
    
    std::function<void()> logout_handler_;
    std::function<void()> profile_handler_;

    std::function<void()> new_project_handler_;
    std::function<void(const std::string&)> new_project_from_template_handler_;
    QString template_directory_;
    std::function<void(const std::string&)> save_project_handler_;
    std::function<void(const std::string&)> load_project_handler_;
    QString current_project_path_;
    std::function<void()> autosave_trigger_handler_;
    std::function<void(const std::string&)> autosave_status_handler_;
    std::function<void()> check_for_updates_handler_;
    std::function<void()> install_update_from_file_handler_;
    std::function<void()> license_activate_handler_;
    std::function<void()> printers_dialog_handler_;
    std::function<void()> send_to_printer_handler_;
    std::function<std::vector<std::string>(const std::string&)> checkpoints_list_provider_;
    std::function<void(const std::string&)> load_checkpoint_handler_;
    std::function<void(const std::string&)> delete_checkpoint_handler_;
    std::function<void(const std::string&, const std::string&)> import_file_handler_;
    std::function<void(const std::string&, const std::string&)> export_file_handler_;
    std::function<void(const std::string&)> command_handler_;
    std::function<UnsavedAction()> ask_unsaved_handler_;
    std::function<std::string()> get_save_path_handler_;
    std::function<void()> property_apply_handler_;
    std::function<void()> property_cancel_handler_;
    std::function<void()> property_apply_and_new_handler_;

    void restoreUiState();
    void saveUiState();
    void showNewProjectDialog();

protected:
    void closeEvent(QCloseEvent* event) override;
};

}  // namespace ui
}  // namespace cad
