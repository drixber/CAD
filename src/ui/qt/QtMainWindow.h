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
#include "core/Modeler/Sketch.h"

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
    QLabel* mode_label_{nullptr};
    QLabel* document_label_{nullptr};
    QLabel* fps_status_label_{nullptr};

    void restoreUiState();
    void saveUiState();
};

}  // namespace ui
}  // namespace cad
