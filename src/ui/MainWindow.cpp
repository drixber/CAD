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
