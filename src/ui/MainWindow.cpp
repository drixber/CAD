#include "MainWindow.h"

namespace cad {
namespace ui {

MainWindow::MainWindow() = default;

void MainWindow::initializeLayout() {
    ribbon_.configureDefaultTabs();
    browser_tree_.setRootLabel("Model");
    property_panel_.setActiveContext("Selection");
    command_line_.setPrompt(">");
}

RibbonLayout& MainWindow::ribbon() {
    return ribbon_;
}

BrowserTree& MainWindow::browserTree() {
    return browser_tree_;
}

PropertyPanel& MainWindow::propertyPanel() {
    return property_panel_;
}

CommandLine& MainWindow::commandLine() {
    return command_line_;
}

void MainWindow::setConstraintCount(int count) {
    (void)count;
#ifdef CAD_USE_QT
    native_window_.setConstraintCount(count);
#endif
}

void MainWindow::setParameterCount(int count) {
    (void)count;
#ifdef CAD_USE_QT
    native_window_.setParameterCount(count);
#endif
}

void MainWindow::setParameterSummary(const std::string& summary) {
    (void)summary;
#ifdef CAD_USE_QT
    native_window_.setParameterSummary(summary);
#endif
}

void MainWindow::setIntegrationStatus(const std::string& status) {
    (void)status;
#ifdef CAD_USE_QT
    native_window_.setIntegrationStatus(status);
#endif
}

void MainWindow::setMateCount(int count) {
    (void)count;
#ifdef CAD_USE_QT
    native_window_.setMateCount(count);
#endif
}

void MainWindow::setCommandHandler(const std::function<void(const std::string&)>& handler) {
    (void)handler;
#ifdef CAD_USE_QT
    native_window_.setCommandHandler(handler);
#endif
}

void MainWindow::setAssemblySummary(const std::string& summary) {
    (void)summary;
#ifdef CAD_USE_QT
    native_window_.setAssemblySummary(summary);
#endif
}

void MainWindow::setMatesSummary(const std::string& summary) {
    (void)summary;
#ifdef CAD_USE_QT
    native_window_.setMatesSummary(summary);
#endif
}

void MainWindow::setContextPlaceholder(const std::string& context) {
    (void)context;
#ifdef CAD_USE_QT
    native_window_.setContextPlaceholder(context);
#endif
}

void MainWindow::appendRecentCommand(const std::string& command) {
    (void)command;
#ifdef CAD_USE_QT
    native_window_.appendRecentCommand(command);
#endif
}

void MainWindow::setContextCategory(const std::string& category) {
    (void)category;
#ifdef CAD_USE_QT
    native_window_.setContextCategory(category);
#endif
}

void MainWindow::setViewportStatus(const std::string& status) {
    (void)status;
#ifdef CAD_USE_QT
    native_window_.setViewportStatus(status);
#endif
}

void MainWindow::setWorkspaceMode(const std::string& mode) {
    (void)mode;
#ifdef CAD_USE_QT
    native_window_.setWorkspaceMode(mode);
#endif
}

void MainWindow::setDocumentLabel(const std::string& label) {
    (void)label;
#ifdef CAD_USE_QT
    native_window_.setDocumentLabel(label);
#endif
}

void MainWindow::setCacheStats(int entries, int max_entries) {
    (void)entries;
    (void)max_entries;
#ifdef CAD_USE_QT
    native_window_.setCacheStats(entries, max_entries);
#endif
}

void MainWindow::setBackgroundLoading(bool enabled) {
    (void)enabled;
#ifdef CAD_USE_QT
    native_window_.setBackgroundLoading(enabled);
#endif
}

bool MainWindow::hasNativeWindow() const {
#ifdef CAD_USE_QT
    return true;
#else
    return false;
#endif
}

#ifdef CAD_USE_QT
QtMainWindow* MainWindow::nativeWindow() {
    return &native_window_;
}
#endif

}  // namespace ui
}  // namespace cad
