#pragma once

#include <functional>

#include "BrowserTree.h"
#include "CommandLine.h"
#include "PropertyPanel.h"
#include "RibbonLayout.h"

#ifdef CAD_USE_QT
#include "qt/QtMainWindow.h"
#endif

namespace cad {
namespace ui {

class MainWindow {
public:
    MainWindow();

    void initializeLayout();

    RibbonLayout& ribbon();
    BrowserTree& browserTree();
    PropertyPanel& propertyPanel();
    CommandLine& commandLine();

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

    bool hasNativeWindow() const;
#ifdef CAD_USE_QT
    QtMainWindow* nativeWindow();
#endif

private:
    RibbonLayout ribbon_;
    BrowserTree browser_tree_;
    PropertyPanel property_panel_;
    CommandLine command_line_;
#ifdef CAD_USE_QT
    QtMainWindow native_window_;
#endif
};

}  // namespace ui
}  // namespace cad
