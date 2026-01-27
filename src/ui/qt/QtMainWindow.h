#pragma once

#include <QMainWindow>
#include <functional>
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
    void setIntegrationStatus(const std::string& status);
    void setMateCount(int count);
    void setCommandHandler(const std::function<void(const std::string&)>& handler);
    void setAssemblySummary(const std::string& summary);
    void setMatesSummary(const std::string& summary);
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
    void setCurrentUser(const std::string& username, const std::string& email);
    void setLogoutHandler(const std::function<void()>& handler);
    
    // Project file operations
    void setSaveProjectHandler(const std::function<void(const std::string&)>& handler);
    void setLoadProjectHandler(const std::function<void(const std::string&)>& handler);
    void setAutoSaveTriggerHandler(const std::function<void()>& handler);
    void setAutoSaveStatusHandler(const std::function<void(const std::string&)>& handler);
    void updateRecentProjectsMenu(const std::vector<std::string>& projects);

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
    QLabel* mode_label_{nullptr};
    QLabel* document_label_{nullptr};
    QLabel* fps_status_label_{nullptr};
    QLabel* autosave_status_label_{nullptr};
    QTimer* autosave_timer_{nullptr};
    QMenu* recent_projects_menu_{nullptr};
    QLabel* user_label_{nullptr};
    QMenu* user_menu_{nullptr};
    
    std::function<void()> logout_handler_;
    
    std::function<void(const std::string&)> save_project_handler_;
    std::function<void(const std::string&)> load_project_handler_;
    std::function<void()> autosave_trigger_handler_;
    std::function<void(const std::string&)> autosave_status_handler_;
    
    void restoreUiState();
    void saveUiState();

protected:
    void closeEvent(QCloseEvent* event) override;
};

}  // namespace ui
}  // namespace cad
