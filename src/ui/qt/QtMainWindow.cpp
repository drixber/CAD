#include "QtMainWindow.h"

#include <QDockWidget>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QWidget>

namespace cad {
namespace ui {

QtMainWindow::QtMainWindow(QWidget* parent)
    : QMainWindow(parent),
      ribbon_(new QtRibbon(this)),
      browser_tree_(new QtBrowserTree(this)),
      property_panel_(new QtPropertyPanel(this)),
      command_line_(new QtCommandLine(this)),
      agent_console_(new QtAgentConsole(this)),
      agent_thoughts_(new QtAgentThoughts(this)),
      viewport_(new QtViewport(this)) {
    QWidget* central = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(central);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(ribbon_);
    layout->addWidget(viewport_);
    setCentralWidget(central);

    ribbon_->setCommandHandler([this](const QString& command) {
        statusBar()->showMessage(tr("Command: %1").arg(command), 2000);
    });

    connect(agent_console_, &QtAgentConsole::commandIssued, this, [this](const QString& command) {
        agent_thoughts_->appendThought(tr("Queued agent task: %1").arg(command));
    });

    QDockWidget* browserDock = new QDockWidget(tr("Model Browser"), this);
    browserDock->setWidget(browser_tree_);
    addDockWidget(Qt::LeftDockWidgetArea, browserDock);

    QDockWidget* propertyDock = new QDockWidget(tr("Properties"), this);
    propertyDock->setWidget(property_panel_);
    addDockWidget(Qt::RightDockWidgetArea, propertyDock);

    QDockWidget* agentDock = new QDockWidget(tr("AI Console"), this);
    agentDock->setWidget(agent_console_);
    addDockWidget(Qt::RightDockWidgetArea, agentDock);

    QDockWidget* thoughtsDock = new QDockWidget(tr("Agent Thoughts"), this);
    thoughtsDock->setWidget(agent_thoughts_);
    addDockWidget(Qt::BottomDockWidgetArea, thoughtsDock);

    statusBar()->addPermanentWidget(command_line_);
}

void QtMainWindow::updateFromSketch(const cad::core::Sketch& sketch) {
    property_panel_->setConstraintCount(static_cast<int>(sketch.constraints().size()));
    property_panel_->setParameterCount(static_cast<int>(sketch.parameters().size()));
}

void QtMainWindow::setConstraintCount(int count) {
    property_panel_->setConstraintCount(count);
}

void QtMainWindow::setParameterCount(int count) {
    property_panel_->setParameterCount(count);
}

void QtMainWindow::setParameterSummary(const std::string& summary) {
    property_panel_->setParameterSummary(QString::fromStdString(summary));
}

void QtMainWindow::setIntegrationStatus(const std::string& status) {
    property_panel_->setIntegrationStatus(QString::fromStdString(status));
}

void QtMainWindow::setMateCount(int count) {
    property_panel_->setMateCount(count);
}

void QtMainWindow::setCommandHandler(const std::function<void(const std::string&)>& handler) {
    ribbon_->setCommandHandler([this, handler](const QString& command) {
        if (handler) {
            handler(command.toStdString());
        }
        statusBar()->showMessage(tr("Command: %1").arg(command), 2000);
    });
}

void QtMainWindow::setAssemblySummary(const std::string& summary) {
    browser_tree_->setAssemblySummary(QString::fromStdString(summary));
}

void QtMainWindow::setMatesSummary(const std::string& summary) {
    browser_tree_->setMatesSummary(QString::fromStdString(summary));
}

}  // namespace ui
}  // namespace cad
