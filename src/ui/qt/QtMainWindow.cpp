#include "QtMainWindow.h"
#include "theme/ThemeManager.h"
#include "theme/DockLayoutManager.h"
#include "QtAIChatPanel.h"
#include <QSettings>
#include <QCloseEvent>

#include <QDockWidget>
#include <QSet>
#include <QSettings>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QWidget>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QKeySequence>
#include <QInputDialog>
#include <QLineEdit>

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
    
    // AI Chat Panel (new modern chat interface)
    ai_chat_panel_ = new QtAIChatPanel(this);
    QDockWidget* aiChatDock = new QDockWidget(tr("AI Chat"), this);
    aiChatDock->setWidget(ai_chat_panel_);
    addDockWidget(Qt::RightDockWidgetArea, aiChatDock);
    aiChatDock->setMinimumWidth(320);
    aiChatDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    tabifyDockWidget(agentDock, aiChatDock);
    aiChatDock->raise(); // Show AI Chat by default

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
    
    // Enable dock widget features for modular arrangement
    browserDock->setFeatures(QDockWidget::DockWidgetMovable | 
                             QDockWidget::DockWidgetFloatable | 
                             QDockWidget::DockWidgetClosable);
    propertyDock->setFeatures(QDockWidget::DockWidgetMovable | 
                             QDockWidget::DockWidgetFloatable | 
                             QDockWidget::DockWidgetClosable);
    agentDock->setFeatures(QDockWidget::DockWidgetMovable | 
                          QDockWidget::DockWidgetFloatable | 
                          QDockWidget::DockWidgetClosable);
    thoughtsDock->setFeatures(QDockWidget::DockWidgetMovable | 
                             QDockWidget::DockWidgetFloatable | 
                             QDockWidget::DockWidgetClosable);
    logDock->setFeatures(QDockWidget::DockWidgetMovable | 
                        QDockWidget::DockWidgetFloatable | 
                        QDockWidget::DockWidgetClosable);
    
    // Initialize layout manager and restore saved layout
    static cad::ui::DockLayoutManager layoutManager;
    QSettings settings("HydraCAD", "HydraCAD");
    layoutManager.loadFromSettings(settings);
    
    // Try to restore default layout, otherwise use current
    if (!layoutManager.restoreDefaultLayout(this)) {
        // Save current layout as default
        layoutManager.saveDefaultLayout(this);
    }
    
    // Layout will be saved in closeEvent

    QDockWidget* perfDock = new QDockWidget(tr("Performance"), this);
    perfDock->setWidget(perf_panel_);
    addDockWidget(Qt::BottomDockWidgetArea, perfDock);
    perfDock->setAllowedAreas(Qt::BottomDockWidgetArea);
    perfDock->setFeatures(QDockWidget::DockWidgetMovable | 
                          QDockWidget::DockWidgetFloatable | 
                          QDockWidget::DockWidgetClosable);
    tabifyDockWidget(logDock, perfDock);

    statusBar()->addPermanentWidget(command_line_);
    mode_label_ = new QLabel(tr("Mode: None"), this);
    document_label_ = new QLabel(tr("Document: Untitled"), this);
    fps_status_label_ = new QLabel(tr("FPS: --"), this);
    autosave_status_label_ = new QLabel(tr("Auto-save: Off"), this);
    statusBar()->addPermanentWidget(mode_label_);
    statusBar()->addPermanentWidget(document_label_);
    
    // Coordinates display
    QLabel* coords_label_ = new QLabel(tr("X: 0.00  Y: 0.00  Z: 0.00"), this);
    statusBar()->addPermanentWidget(coords_label_);
    
    // Units display
    QLabel* units_label_ = new QLabel(tr("Units: mm"), this);
    statusBar()->addPermanentWidget(units_label_);
    
    // Snap mode
    QLabel* snap_label_ = new QLabel(tr("Snap: Off"), this);
    statusBar()->addPermanentWidget(snap_label_);
    
    statusBar()->addPermanentWidget(fps_status_label_);
    statusBar()->addPermanentWidget(autosave_status_label_);
    
    user_label_ = new QLabel(tr("User: Not logged in"), this);
    statusBar()->addPermanentWidget(user_label_);
    
    // Create menu bar with File menu
    QMenuBar* menu_bar = menuBar();
    // User menu
    user_menu_ = menu_bar->addMenu(tr("&User"));
    QAction* profile_action = user_menu_->addAction(tr("&Profile"));
    QAction* logout_action = user_menu_->addAction(tr("&Logout"));
    connect(logout_action, &QAction::triggered, this, [this]() {
        if (logout_handler_) {
            logout_handler_();
        }
    });
    
    // Layout menu
    QMenu* layout_menu = menu_bar->addMenu(tr("&Layout"));
    DockLayoutManager* layout_manager = new DockLayoutManager();
    
    QAction* save_layout_action = layout_menu->addAction(tr("&Save Current Layout..."), this, [this, layout_manager]() {
        bool ok;
        QString name = QInputDialog::getText(this, tr("Save Layout"), tr("Layout name:"), 
                                            QLineEdit::Normal, "", &ok);
        if (ok && !name.isEmpty()) {
            layout_manager->saveLayout(this, name);
            QSettings settings;
            layout_manager->saveToSettings(settings);
        }
    });
    
    layout_menu->addSeparator();
    
    QMenu* templates_menu = layout_menu->addMenu(tr("&Templates"));
    QAction* inventor_layout = templates_menu->addAction(tr("&Inventor Style"), this, [this, layout_manager]() {
        layout_manager->applyInventorLayout(this);
    });
    QAction* solidworks_layout = templates_menu->addAction(tr("&SolidWorks Style"), this, [this, layout_manager]() {
        layout_manager->applySolidWorksLayout(this);
    });
    QAction* catia_layout = templates_menu->addAction(tr("&CATIA Style"), this, [this, layout_manager]() {
        layout_manager->applyCATIALayout(this);
    });
    QAction* default_layout = templates_menu->addAction(tr("&Default"), this, [this, layout_manager]() {
        layout_manager->restoreDefaultLayout(this);
    });
    
    layout_menu->addSeparator();
    
    QMenu* saved_layouts_menu = layout_menu->addMenu(tr("&Saved Layouts"));
    // Populate saved layouts
    QStringList saved = layout_manager->getSavedLayouts();
    for (const QString& name : saved) {
        QAction* action = saved_layouts_menu->addAction(name, this, [this, layout_manager, name]() {
            layout_manager->restoreLayout(this, name);
        });
    }
    
    QMenu* file_menu = menu_bar->addMenu(tr("&File"));
    
    QAction* new_action = file_menu->addAction(tr("&New Project"), this, [this]() {
        log_panel_->appendLog(tr("New Project"));
        setDocumentLabel("Untitled");
    });
    new_action->setShortcut(QKeySequence::New);
    
    QAction* open_action = file_menu->addAction(tr("&Open Project..."), this, [this]() {
        QString file_path = QFileDialog::getOpenFileName(this, tr("Open Project"), 
                                                       "", tr("CAD Project (*.cad)"));
        if (!file_path.isEmpty() && load_project_handler_) {
            load_project_handler_(file_path.toStdString());
        }
    });
    open_action->setShortcut(QKeySequence::Open);
    
    file_menu->addSeparator();
    
    QAction* save_action = file_menu->addAction(tr("&Save Project"), this, [this]() {
        QString file_path = QFileDialog::getSaveFileName(this, tr("Save Project"), 
                                                        "", tr("CAD Project (*.cad)"));
        if (!file_path.isEmpty() && save_project_handler_) {
            save_project_handler_(file_path.toStdString());
        }
    });
    save_action->setShortcut(QKeySequence::Save);
    
    QAction* save_as_action = file_menu->addAction(tr("Save Project &As..."), this, [this]() {
        QString file_path = QFileDialog::getSaveFileName(this, tr("Save Project As"), 
                                                        "", tr("CAD Project (*.cad)"));
        if (!file_path.isEmpty() && save_project_handler_) {
            save_project_handler_(file_path.toStdString());
        }
    });
    save_as_action->setShortcut(QKeySequence::SaveAs);
    
    file_menu->addSeparator();
    
    recent_projects_menu_ = file_menu->addMenu(tr("Recent Projects"));
    // Recent projects will be populated dynamically
    
    file_menu->addSeparator();
    QAction* exit_action = file_menu->addAction(tr("E&xit"), this, &QMainWindow::close);
    exit_action->setShortcut(QKeySequence::Quit);
    
    // Setup auto-save timer
    autosave_timer_ = new QTimer(this);
    autosave_timer_->setInterval(300000);  // 5 minutes in milliseconds
    connect(autosave_timer_, &QTimer::timeout, this, [this]() {
        if (autosave_trigger_handler_) {
            autosave_trigger_handler_();
            if (autosave_status_handler_) {
                autosave_status_handler_("Auto-saved");
            }
            if (autosave_status_label_) {
                autosave_status_label_->setText(tr("Auto-save: Saved"));
                QTimer::singleShot(3000, this, [this]() {
                    if (autosave_status_label_) {
                        autosave_status_label_->setText(tr("Auto-save: On"));
                    }
                });
            }
        }
    });
    autosave_timer_->start();
    if (autosave_status_label_) {
        autosave_status_label_->setText(tr("Auto-save: On"));
    }

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
    QSettings settings("HydraCAD", "HydraCAD");
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
    QSettings settings("HydraCAD", "HydraCAD");
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

void QtMainWindow::setSaveProjectHandler(const std::function<void(const std::string&)>& handler) {
    save_project_handler_ = handler;
}

void QtMainWindow::setLoadProjectHandler(const std::function<void(const std::string&)>& handler) {
    load_project_handler_ = handler;
}

void QtMainWindow::setAutoSaveTriggerHandler(const std::function<void()>& handler) {
    autosave_trigger_handler_ = handler;
    if (autosave_timer_ && handler) {
        autosave_timer_->start();
        if (autosave_status_label_) {
            autosave_status_label_->setText(tr("Auto-save: On"));
        }
    } else if (autosave_timer_) {
        autosave_timer_->stop();
        if (autosave_status_label_) {
            autosave_status_label_->setText(tr("Auto-save: Off"));
        }
    }
}

void QtMainWindow::setAutoSaveStatusHandler(const std::function<void(const std::string&)>& handler) {
    autosave_status_handler_ = handler;
}

void QtMainWindow::setCurrentUser(const std::string& username, const std::string& email) {
    if (user_label_) {
        QString user_text = tr("User: %1").arg(QString::fromStdString(username));
        if (!email.empty()) {
            user_text += tr(" (%1)").arg(QString::fromStdString(email));
        }
        user_label_->setText(user_text);
    }
}

void QtMainWindow::setLogoutHandler(const std::function<void()>& handler) {
    logout_handler_ = handler;
}

void QtMainWindow::updateRecentProjectsMenu(const std::vector<std::string>& projects) {
    if (!recent_projects_menu_) {
        return;
    }
    recent_projects_menu_->clear();
    for (const auto& project : projects) {
        QAction* action = recent_projects_menu_->addAction(QString::fromStdString(project));
        connect(action, &QAction::triggered, this, [this, project]() {
            if (load_project_handler_) {
                load_project_handler_(project);
            }
        });
    }
    if (projects.empty()) {
        QAction* no_projects = recent_projects_menu_->addAction(tr("No recent projects"));
        no_projects->setEnabled(false);
    }
}

void QtMainWindow::closeEvent(QCloseEvent* event) {
    // Save layout before closing
    static cad::ui::DockLayoutManager layoutManager;
    layoutManager.saveDefaultLayout(this);
    QSettings settings("HydraCAD", "HydraCAD");
    layoutManager.saveToSettings(settings);
    
    QMainWindow::closeEvent(event);
}

}  // namespace ui
}  // namespace cad
