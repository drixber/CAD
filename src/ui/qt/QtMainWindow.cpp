#include "QtMainWindow.h"

#include <QDockWidget>
#include <QSet>
#include <QSettings>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QWidget>

namespace cad {
namespace ui {

namespace {

QString categoryForCommand(const QString& command) {
    const QSet<QString> sketch = {"Line", "Rectangle", "Circle", "Arc", "Constraint"};
    const QSet<QString> part = {"Extrude", "Revolve", "Loft", "Hole", "Fillet",
                                "Flange", "Bend", "Unfold", "Refold",
                                "RectangularPattern", "CircularPattern", "CurvePattern",
                                "DirectEdit", "Freeform"};
    const QSet<QString> assembly = {"Place", "Mate", "Flush", "Angle", "Pattern",
                                    "RigidPipe", "FlexibleHose", "BentTube", "Simplify"};
    const QSet<QString> drawing = {"BaseView", "Section", "Dimension", "PartsList"};
    const QSet<QString> inspect = {"Measure", "Interference", "SectionAnalysis",
                                   "Simulation", "StressAnalysis"};
    const QSet<QString> manage = {"Parameters", "Styles", "AddIns", "Import",
                                  "Export", "ExportRFA", "MbdNote"};
    const QSet<QString> view = {"Visibility", "Appearance", "Environment",
                                "Illustration", "Rendering", "Animation"};

    if (sketch.contains(command)) {
        return "Sketch";
    }
    if (part.contains(command)) {
        return "Part";
    }
    if (assembly.contains(command)) {
        return "Assembly";
    }
    if (drawing.contains(command)) {
        return "Drawing";
    }
    if (inspect.contains(command)) {
        return "Inspect";
    }
    if (manage.contains(command)) {
        return "Manage";
    }
    if (view.contains(command)) {
        return "View";
    }
    return "General";
}

}  // namespace

QtMainWindow::QtMainWindow(QWidget* parent)
    : QMainWindow(parent),
      ribbon_(new QtRibbon(this)),
      browser_tree_(new QtBrowserTree(this)),
      property_panel_(new QtPropertyPanel(this)),
      command_line_(new QtCommandLine(this)),
      agent_console_(new QtAgentConsole(this)),
      agent_thoughts_(new QtAgentThoughts(this)),
      viewport_(new QtViewport(this)),
      log_panel_(new QtLogPanel(this)),
      perf_panel_(new QtPerformancePanel(this)) {
    QWidget* central = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(central);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(ribbon_);
    layout->addWidget(viewport_);
    setCentralWidget(central);

    ribbon_->setCommandHandler([this](const QString& command) {
        statusBar()->showMessage(tr("Command: %1").arg(command), 2000);
        command_line_->setText(command);
        QStringList history = command_line_->history();
        history.append(command);
        if (history.size() > 20) {
            history.removeFirst();
        }
        command_line_->setHistory(history);
        property_panel_->setContextPlaceholder(command);
        property_panel_->setContextCategory(categoryForCommand(command));
        setWorkspaceMode(categoryForCommand(command).toStdString());
        browser_tree_->appendRecentCommand(command);
        log_panel_->appendLog(tr("Command: %1").arg(command));
    });

    connect(ribbon_, &QTabWidget::currentChanged, this, [this](int index) {
        const QString tab = ribbon_->tabText(index);
        if (tab.isEmpty()) {
            return;
        }
        property_panel_->setContextCategory(tab);
        setWorkspaceMode(tab.toStdString());
        setViewportStatus(QString("Workspace: %1").arg(tab).toStdString());
        if (tab == tr("Drawing")) {
            log_panel_->appendLog(tr("Drawing workspace active"));
        }
        log_panel_->appendLog(tr("Workspace changed: %1").arg(tab));
    });

    connect(agent_console_, &QtAgentConsole::commandIssued, this, [this](const QString& command) {
        agent_thoughts_->appendThought(tr("Queued agent task: %1").arg(command));
        command_line_->setText(command);
        QStringList history = command_line_->history();
        history.append(command);
        if (history.size() > 20) {
            history.removeFirst();
        }
        command_line_->setHistory(history);
        property_panel_->setContextPlaceholder(command);
        property_panel_->setContextCategory(categoryForCommand(command));
        setWorkspaceMode(categoryForCommand(command).toStdString());
        browser_tree_->appendRecentCommand(command);
        log_panel_->appendLog(tr("Agent: %1").arg(command));
    });

    connect(command_line_, &QLineEdit::returnPressed, this, [this]() {
        const QString command = command_line_->takeCurrentCommand();
        if (command.isEmpty()) {
            return;
        }
        statusBar()->showMessage(tr("Command: %1").arg(command), 2000);
        property_panel_->setContextPlaceholder(command);
        property_panel_->setContextCategory(categoryForCommand(command));
        setWorkspaceMode(categoryForCommand(command).toStdString());
        browser_tree_->appendRecentCommand(command);
        log_panel_->appendLog(tr("Command Line: %1").arg(command));
    });

    QDockWidget* browserDock = new QDockWidget(tr("Model Browser"), this);
    browserDock->setWidget(browser_tree_);
    addDockWidget(Qt::LeftDockWidgetArea, browserDock);
    browserDock->setMinimumWidth(220);

    QDockWidget* propertyDock = new QDockWidget(tr("Properties"), this);
    propertyDock->setWidget(property_panel_);
    addDockWidget(Qt::RightDockWidgetArea, propertyDock);
    propertyDock->setMinimumWidth(280);
    propertyDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    QDockWidget* agentDock = new QDockWidget(tr("AI Console"), this);
    agentDock->setWidget(agent_console_);
    addDockWidget(Qt::RightDockWidgetArea, agentDock);
    agentDock->setMinimumWidth(280);
    agentDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    QDockWidget* thoughtsDock = new QDockWidget(tr("Agent Thoughts"), this);
    thoughtsDock->setWidget(agent_thoughts_);
    addDockWidget(Qt::BottomDockWidgetArea, thoughtsDock);
    thoughtsDock->setAllowedAreas(Qt::BottomDockWidgetArea);

    QDockWidget* logDock = new QDockWidget(tr("Log"), this);
    logDock->setWidget(log_panel_);
    addDockWidget(Qt::BottomDockWidgetArea, logDock);
    logDock->setAllowedAreas(Qt::BottomDockWidgetArea);
    tabifyDockWidget(thoughtsDock, logDock);
    logDock->raise();

    tabifyDockWidget(propertyDock, agentDock);
    propertyDock->raise();

    QDockWidget* perfDock = new QDockWidget(tr("Performance"), this);
    perfDock->setWidget(perf_panel_);
    addDockWidget(Qt::BottomDockWidgetArea, perfDock);
    perfDock->setAllowedAreas(Qt::BottomDockWidgetArea);
    tabifyDockWidget(logDock, perfDock);

    statusBar()->addPermanentWidget(command_line_);
    mode_label_ = new QLabel(tr("Mode: None"), this);
    document_label_ = new QLabel(tr("Document: Untitled"), this);
    fps_status_label_ = new QLabel(tr("FPS: --"), this);
    statusBar()->addPermanentWidget(mode_label_);
    statusBar()->addPermanentWidget(document_label_);
    statusBar()->addPermanentWidget(fps_status_label_);

    connect(viewport_, &QtViewport::fpsUpdated, this, [this](double fps) {
        if (fps_status_label_) {
            fps_status_label_->setText(tr("FPS: %1").arg(QString::number(fps, 'f', 0)));
        }
        if (perf_panel_) {
            perf_panel_->setFps(fps);
        }
    });

    connect(perf_panel_, &QtPerformancePanel::lodModeChanged, this, [this](const QString& mode) {
        log_panel_->appendLog(tr("LOD mode: %1").arg(mode));
        setViewportStatus(QString("LOD: %1").arg(mode).toStdString());
        setWorkspaceMode("Assembly");
    });

    connect(perf_panel_, &QtPerformancePanel::backgroundLoadingToggled, this, [this](bool enabled) {
        log_panel_->appendLog(tr("Background loading: %1").arg(enabled ? tr("on") : tr("off")));
        setViewportStatus(enabled ? "Background loading enabled" : "Background loading disabled");
    });

    connect(perf_panel_, &QtPerformancePanel::targetFpsChanged, this, [this](int fps) {
        log_panel_->appendLog(tr("Target FPS: %1").arg(fps));
        setViewportStatus("Target FPS " + std::to_string(fps));
        if (fps_status_label_) {
            fps_status_label_->setText(tr("Target FPS: %1").arg(fps));
        }
    });

    restoreUiState();
}

QtMainWindow::~QtMainWindow() {
    saveUiState();
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
        command_line_->setText(command);
        QStringList history = command_line_->history();
        history.append(command);
        if (history.size() > 20) {
            history.removeFirst();
        }
        command_line_->setHistory(history);
        property_panel_->setContextPlaceholder(command);
        property_panel_->setContextCategory(categoryForCommand(command));
        setWorkspaceMode(categoryForCommand(command).toStdString());
        browser_tree_->appendRecentCommand(command);
        log_panel_->appendLog(tr("Command: %1").arg(command));
    });
}

void QtMainWindow::setAssemblySummary(const std::string& summary) {
    browser_tree_->setAssemblySummary(QString::fromStdString(summary));
}

void QtMainWindow::setMatesSummary(const std::string& summary) {
    browser_tree_->setMatesSummary(QString::fromStdString(summary));
}

void QtMainWindow::setContextPlaceholder(const std::string& context) {
    property_panel_->setContextPlaceholder(QString::fromStdString(context));
}

void QtMainWindow::appendRecentCommand(const std::string& command) {
    browser_tree_->appendRecentCommand(QString::fromStdString(command));
}

void QtMainWindow::setContextCategory(const std::string& category) {
    property_panel_->setContextCategory(QString::fromStdString(category));
}

void QtMainWindow::setViewportStatus(const std::string& status) {
    viewport_->setStatusText(QString::fromStdString(status));
}

void QtMainWindow::setWorkspaceMode(const std::string& mode) {
    if (mode_label_) {
        mode_label_->setText(tr("Mode: %1").arg(QString::fromStdString(mode)));
    }
}

void QtMainWindow::setDocumentLabel(const std::string& label) {
    if (document_label_) {
        document_label_->setText(tr("Document: %1").arg(QString::fromStdString(label)));
    }
}

void QtMainWindow::setCacheStats(int entries, int max_entries) {
    if (perf_panel_) {
        perf_panel_->setCacheStats(entries, max_entries);
    }
}

void QtMainWindow::setBackgroundLoading(bool enabled) {
    if (perf_panel_) {
        perf_panel_->setBackgroundLoading(enabled);
    }
}

void QtMainWindow::setLoadProgress(int progress) {
    if (perf_panel_) {
        perf_panel_->setProgress(progress);
    }
}

void QtMainWindow::setTargetFps(int fps) {
    if (perf_panel_) {
        perf_panel_->setTargetFps(fps);
    }
}

void QtMainWindow::setTargetFpsHandler(const std::function<void(int)>& handler) {
    connect(perf_panel_, &QtPerformancePanel::targetFpsChanged, this, [handler](int fps) {
        if (handler) {
            handler(fps);
        }
    });
}

void QtMainWindow::setLodModeHandler(const std::function<void(const std::string&)>& handler) {
    connect(perf_panel_, &QtPerformancePanel::lodModeChanged, this, [this, handler](const QString& mode) {
        if (handler) {
            handler(mode.toStdString());
        }
    });
}

void QtMainWindow::setBackgroundLoadingHandler(const std::function<void(bool)>& handler) {
    connect(perf_panel_, &QtPerformancePanel::backgroundLoadingToggled, this, [this, handler](bool enabled) {
        if (handler) {
            handler(enabled);
        }
    });
}

void QtMainWindow::restoreUiState() {
    QSettings settings("CADursor", "CADursor");
    const QByteArray geometry = settings.value("mainWindow/geometry").toByteArray();
    if (!geometry.isEmpty()) {
        restoreGeometry(geometry);
    }
    const QByteArray state = settings.value("mainWindow/state").toByteArray();
    if (!state.isEmpty()) {
        restoreState(state);
    }
}

void QtMainWindow::saveUiState() {
    QSettings settings("CADursor", "CADursor");
    settings.setValue("mainWindow/geometry", saveGeometry());
    settings.setValue("mainWindow/state", saveState());
}

QtPropertyPanel* QtMainWindow::propertyPanel() {
    return property_panel_;
}

QtCommandLine* QtMainWindow::commandLine() {
    return command_line_;
}

Viewport3D* QtMainWindow::viewport3D() {
    if (viewport_) {
        return viewport_->viewport3D();
    }
    return nullptr;
}

}  // namespace ui
}  // namespace cad
