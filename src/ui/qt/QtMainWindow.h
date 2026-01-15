#pragma once

#include <QMainWindow>
#include <functional>

#include "QtBrowserTree.h"
#include "QtCommandLine.h"
#include "QtPropertyPanel.h"
#include "QtRibbon.h"
#include "QtAgentConsole.h"
#include "QtAgentThoughts.h"
#include "QtViewport.h"
#include "core/Modeler/Sketch.h"

namespace cad {
namespace ui {

class QtMainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit QtMainWindow(QWidget* parent = nullptr);
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

private:
    QtRibbon* ribbon_{nullptr};
    QtBrowserTree* browser_tree_{nullptr};
    QtPropertyPanel* property_panel_{nullptr};
    QtCommandLine* command_line_{nullptr};
    QtAgentConsole* agent_console_{nullptr};
    QtAgentThoughts* agent_thoughts_{nullptr};
    QtViewport* viewport_{nullptr};
};

}  // namespace ui
}  // namespace cad
