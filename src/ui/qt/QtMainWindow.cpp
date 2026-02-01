#include "QtMainWindow.h"
#include "theme/ThemeManager.h"
#include "theme/DockLayoutManager.h"
#include "QtAIChatPanel.h"
#include "QtCommunityPanel.h"
#include <QSettings>
#include <QCloseEvent>

#include <QDockWidget>
#include <QSet>
#include <QStatusBar>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QWidget>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QActionGroup>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QKeySequence>
#include <QShortcut>
#include <QInputDialog>
#include <QLineEdit>
#include <QListWidget>
#include <QLabel>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QDialog>
#include <QStandardPaths>
#include <QTextStream>
#include <QToolBar>
#include <QUrl>

namespace cad {
namespace ui {

namespace {

QString categoryForCommand(const QString& command) {
    const QSet<QString> sketch = {"Line", "Rectangle", "Circle", "Arc", "Constraint"};
    const QSet<QString> part = {"Extrude", "ExtrudeReverse", "ExtrudeBoth", "Revolve", "Loft", "Hole", "HoleThroughAll", "Fillet", "Chamfer", "Shell", "Mirror",
                                "Flange", "Bend", "Unfold", "Refold", "Punch", "Bead", "SheetMetalRules", "ExportFlatDXF",
                                "RectangularPattern", "CircularPattern", "CurvePattern", "FacePattern",
                                "DirectEdit", "Freeform"};
    const QSet<QString> assembly = {"Place", "Mate", "Flush", "Angle", "Parallel", "Distance", "Pattern",
                                    "RigidPipe", "FlexibleHose", "BentTube", "RouteBOM", "Weld", "WeldBOM", "Simplify"};
    const QSet<QString> drawing = {"BaseView", "Section", "Dimension", "PartsList"};
    const QSet<QString> inspect = {"Measure", "Interference", "SectionAnalysis",
                                   "Simulation", "StressAnalysis", "ExportFEAReport", "ExportMotionReport"};
    const QSet<QString> manage = {"Parameters", "iLogic", "Styles", "AddIns", "Import",
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

QString logsDirectory() {
    const QString app_data = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (app_data.isEmpty()) {
        return QString();
    }
    return QDir(app_data).filePath("logs");
}

QString readLogPreview(const QString& path, int max_lines = 200) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QString();
    }
    QTextStream in(&file);
    QStringList lines;
    while (!in.atEnd()) {
        lines.append(in.readLine());
        if (lines.size() > max_lines) {
            lines.removeFirst();
        }
    }
    return lines.join('\n');
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
    central->setObjectName("centralWorkspace");
    QVBoxLayout* layout = new QVBoxLayout(central);
    layout->setContentsMargins(2, 2, 2, 2);
    QWidget* ribbon_container = new QWidget(this);
    QVBoxLayout* ribbon_v = new QVBoxLayout(ribbon_container);
    ribbon_v->setContentsMargins(0, 0, 0, 0);
    QHBoxLayout* ribbon_row = new QHBoxLayout();
    ribbon_row->addWidget(ribbon_, 1);
    if (ribbon_->searchLineEdit()) {
        ribbon_row->addWidget(ribbon_->searchLineEdit(), 0, Qt::AlignVCenter);
    }
    ribbon_v->addLayout(ribbon_row);
    if (ribbon_->documentTabBar()) {
        ribbon_v->addWidget(ribbon_->documentTabBar());
    }
    layout->addWidget(ribbon_container);
    layout->addWidget(viewport_, 1);  // stretch so viewport gets remaining space
    setCentralWidget(central);

    // Quick-access toolbar (File: New, Open, Save)
    QToolBar* file_toolbar = addToolBar(tr("File"));
    file_toolbar->setObjectName("fileToolbar");
    file_toolbar->setMovable(false);
    QAction* tb_new = file_toolbar->addAction(tr("New"));
    QAction* tb_open = file_toolbar->addAction(tr("Open"));
    QAction* tb_save = file_toolbar->addAction(tr("Save"));
    connect(tb_new, &QAction::triggered, this, [this]() { showNewProjectDialog(); });
    connect(tb_open, &QAction::triggered, this, [this]() {
        QString path = QFileDialog::getOpenFileName(this, tr("Open Project"), "", tr("CAD Project (*.cad);;All Files (*)"));
        if (!path.isEmpty() && load_project_handler_) { load_project_handler_(path.toStdString()); }
    });
    connect(tb_save, &QAction::triggered, this, [this]() {
        if (!current_project_path_.isEmpty() && save_project_handler_) {
            save_project_handler_(current_project_path_.toStdString());
            statusBar()->showMessage(tr("Project saved."), 3000);
        } else {
            QString path = QFileDialog::getSaveFileName(this, tr("Save Project"), "", tr("CAD Project (*.cad);;All Files (*)"));
            if (!path.isEmpty() && save_project_handler_) { save_project_handler_(path.toStdString()); }
        }
    });

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

    new QShortcut(QKeySequence::Undo, this, [this]() { executeCommand("Undo"); });
    new QShortcut(QKeySequence::Redo, this, [this]() { executeCommand("Redo"); });

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
        if (command_handler_) {
            command_handler_(command.toStdString());
        }
        statusBar()->showMessage(tr("Command: %1").arg(command), 2000);
        property_panel_->setContextPlaceholder(command);
        property_panel_->setContextCategory(categoryForCommand(command));
        setWorkspaceMode(categoryForCommand(command).toStdString());
        browser_tree_->appendRecentCommand(command);
        log_panel_->appendLog(tr("Command Line: %1").arg(command));
    });

    QDockWidget* browserDock = new QDockWidget(tr("Model Browser"), this);
    browserDock->setObjectName("dock_model_browser");
    QWidget* browser_wrapper = new QWidget(this);
    QVBoxLayout* browser_layout = new QVBoxLayout(browser_wrapper);
    browser_layout->setContentsMargins(2, 2, 2, 2);
    QHBoxLayout* browser_title = new QHBoxLayout();
    QLineEdit* browser_filter = new QLineEdit(browser_wrapper);
    browser_filter->setPlaceholderText(tr("Filter..."));
    browser_filter->setClearButtonEnabled(true);
    browser_filter->setMaximumHeight(24);
    QPushButton* browser_add_btn = new QPushButton("+", browser_wrapper);
    browser_add_btn->setToolTip(tr("Add"));
    browser_add_btn->setFixedSize(24, 24);
    browser_title->addWidget(browser_filter, 1);
    browser_title->addWidget(browser_add_btn, 0);
    browser_layout->addLayout(browser_title);
    browser_layout->addWidget(browser_tree_, 1);
    connect(browser_filter, &QLineEdit::textChanged, this, [this](const QString& text) {
        browser_tree_->setFilterText(text);
    });
    connect(browser_add_btn, &QPushButton::clicked, this, [this]() {
        executeCommand("Create New Component");
        if (statusBar()) statusBar()->showMessage(tr("Create New Component"), 2000);
    });
    browserDock->setWidget(browser_wrapper);
    addDockWidget(Qt::LeftDockWidgetArea, browserDock);
    browserDock->setMinimumWidth(220);

    QDockWidget* propertyDock = new QDockWidget(tr("Properties"), this);
    propertyDock->setObjectName("dock_properties");
    propertyDock->setWidget(property_panel_);
    addDockWidget(Qt::RightDockWidgetArea, propertyDock);
    propertyDock->setMinimumWidth(280);
    propertyDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    connect(property_panel_, &QtPropertyPanel::closePanelRequested, propertyDock, &QDockWidget::hide);
    connect(property_panel_, &QtPropertyPanel::applyRequested, this, [this]() {
        if (property_apply_handler_) property_apply_handler_();
        if (statusBar()) statusBar()->showMessage(tr("Properties applied."), 2000);
    });
    connect(property_panel_, &QtPropertyPanel::cancelRequested, this, [this]() {
        if (property_cancel_handler_) property_cancel_handler_();
        if (statusBar()) statusBar()->showMessage(tr("Properties cancelled."), 2000);
    });
    connect(property_panel_, &QtPropertyPanel::applyAndNewRequested, this, [this]() {
        if (property_apply_and_new_handler_) property_apply_and_new_handler_();
        if (statusBar()) statusBar()->showMessage(tr("Applied & New – ready for next feature."), 2000);
    });
    connect(property_panel_, &QtPropertyPanel::visibilityToggled, this, [this]() {
        if (statusBar()) statusBar()->showMessage(tr("Visibility toggled."), 1500);
    });
    connect(property_panel_, &QtPropertyPanel::newConfigurationRequested, this, [this]() {
        if (statusBar()) statusBar()->showMessage(tr("New configuration."), 1500);
    });

    QDockWidget* agentDock = new QDockWidget(tr("AI Console"), this);
    agentDock->setObjectName("dock_ai_console");
    agentDock->setWidget(agent_console_);
    addDockWidget(Qt::RightDockWidgetArea, agentDock);
    agentDock->setMinimumWidth(280);
    agentDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    
    // AI Chat Panel (new modern chat interface)
    ai_chat_panel_ = new QtAIChatPanel(this);
    QDockWidget* aiChatDock = new QDockWidget(tr("AI Chat"), this);
    aiChatDock->setObjectName("dock_ai_chat");
    aiChatDock->setWidget(ai_chat_panel_);
    addDockWidget(Qt::RightDockWidgetArea, aiChatDock);
    aiChatDock->setMinimumWidth(320);
    aiChatDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    community_panel_ = new QtCommunityPanel(this);
    QDockWidget* communityDock = new QDockWidget(tr("Community"), this);
    communityDock->setObjectName("dock_community");
    communityDock->setWidget(community_panel_);
    addDockWidget(Qt::RightDockWidgetArea, communityDock);
    communityDock->setMinimumWidth(320);
    communityDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    tabifyDockWidget(aiChatDock, communityDock);
    communityDock->setFeatures(QDockWidget::DockWidgetMovable |
                              QDockWidget::DockWidgetFloatable |
                              QDockWidget::DockWidgetClosable);
    communityDock->setVisible(false);  // Offline mode: Community (cloud) hidden by default

    tabifyDockWidget(agentDock, aiChatDock);
    aiChatDock->raise(); // Show AI Chat by default

    QDockWidget* thoughtsDock = new QDockWidget(tr("Agent Thoughts"), this);
    thoughtsDock->setObjectName("dock_agent_thoughts");
    thoughtsDock->setWidget(agent_thoughts_);
    addDockWidget(Qt::BottomDockWidgetArea, thoughtsDock);
    thoughtsDock->setAllowedAreas(Qt::BottomDockWidgetArea);

    QDockWidget* logDock = new QDockWidget(tr("Log"), this);
    logDock->setObjectName("dock_log");
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
    perfDock->setObjectName("dock_performance");
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
    
    user_label_ = new QLabel(tr("Offline"), this);
    statusBar()->addPermanentWidget(user_label_);

    // Create menu bar with File menu (no User menu in offline mode)
    QMenuBar* menu_bar = menuBar();

    // Layout menu – use the same static layoutManager; explicit connect() so actions always fire
    QMenu* layout_menu = menu_bar->addMenu(tr("&Layout"));
    DockLayoutManager* layout_mgr = &layoutManager;

    QAction* save_layout_act = layout_menu->addAction(tr("&Save Current Layout..."));
    connect(save_layout_act, &QAction::triggered, this, [this, layout_mgr]() {
        bool ok;
        QString name = QInputDialog::getText(this, tr("Save Layout"), tr("Layout name:"),
                                            QLineEdit::Normal, "", &ok);
        if (ok && !name.isEmpty()) {
            layout_mgr->saveLayout(this, name);
            QSettings settings("HydraCAD", "HydraCAD");
            layout_mgr->saveToSettings(settings);
            if (statusBar()) statusBar()->showMessage(tr("Layout \"%1\" saved.").arg(name), 3000);
        }
    });

    layout_menu->addSeparator();

    QMenu* templates_menu = layout_menu->addMenu(tr("&Templates"));
    QAction* inventor_act = templates_menu->addAction(tr("&Inventor Style"));
    connect(inventor_act, &QAction::triggered, this, [this, layout_mgr]() {
        layout_mgr->applyInventorLayout(this);
        if (statusBar()) statusBar()->showMessage(tr("Layout: Inventor Style"), 2000);
    });
    QAction* solidworks_act = templates_menu->addAction(tr("&SolidWorks Style"));
    connect(solidworks_act, &QAction::triggered, this, [this, layout_mgr]() {
        layout_mgr->applySolidWorksLayout(this);
        if (statusBar()) statusBar()->showMessage(tr("Layout: SolidWorks Style"), 2000);
    });
    QAction* catia_act = templates_menu->addAction(tr("&CATIA Style"));
    connect(catia_act, &QAction::triggered, this, [this, layout_mgr]() {
        layout_mgr->applyCATIALayout(this);
        if (statusBar()) statusBar()->showMessage(tr("Layout: CATIA Style"), 2000);
    });
    QAction* default_act = templates_menu->addAction(tr("&Default"));
    connect(default_act, &QAction::triggered, this, [this, layout_mgr]() {
        if (layout_mgr->restoreDefaultLayout(this)) {
            if (statusBar()) statusBar()->showMessage(tr("Default layout restored."), 2000);
        }
    });

    layout_menu->addSeparator();

    QMenu* saved_layouts_menu = layout_menu->addMenu(tr("&Saved Layouts"));
    connect(saved_layouts_menu, &QMenu::aboutToShow, this, [this, saved_layouts_menu, layout_mgr]() {
        saved_layouts_menu->clear();
        for (const QString& name : layout_mgr->getSavedLayouts()) {
            QAction* act = saved_layouts_menu->addAction(name);
            connect(act, &QAction::triggered, this, [this, layout_mgr, name]() {
                if (layout_mgr->restoreLayout(this, name)) {
                    if (statusBar()) statusBar()->showMessage(tr("Layout \"%1\" restored.").arg(name), 2000);
                }
            });
        }
        if (saved_layouts_menu->actions().isEmpty()) {
            QAction* none = saved_layouts_menu->addAction(tr("(No saved layouts)"));
            none->setEnabled(false);
        }
    });
    
    QMenu* file_menu = menu_bar->addMenu(tr("&File"));
    
    QAction* new_action = file_menu->addAction(tr("&New Project"), this, [this]() {
        log_panel_->appendLog(tr("New Project"));
        showNewProjectDialog();
    });
    new_action->setShortcut(QKeySequence::New);
    
    QAction* open_action = file_menu->addAction(tr("&Open Project..."), this, [this]() {
        QString file_path = QFileDialog::getOpenFileName(this, tr("Open Project"), 
                                                       "", tr("CAD Project (*.cad);;All Files (*)"));
        if (!file_path.isEmpty() && load_project_handler_) {
            load_project_handler_(file_path.toStdString());
        }
    });
    open_action->setShortcut(QKeySequence::Open);
    
    file_menu->addSeparator();
    
    QAction* save_action = file_menu->addAction(tr("&Save Project"), this, [this]() {
        if (!current_project_path_.isEmpty() && save_project_handler_) {
            save_project_handler_(current_project_path_.toStdString());
            statusBar()->showMessage(tr("Project saved."), 3000);
        } else {
            QString file_path = QFileDialog::getSaveFileName(this, tr("Save Project"), 
                                                            "", tr("CAD Project (*.cad);;All Files (*)"));
            if (!file_path.isEmpty() && save_project_handler_) {
                save_project_handler_(file_path.toStdString());
            }
        }
    });
    save_action->setShortcut(QKeySequence::Save);
    
    QAction* save_as_action = file_menu->addAction(tr("Save Project &As..."), this, [this]() {
        QString file_path = QFileDialog::getSaveFileName(this, tr("Save Project As"),
                                                        "", tr("CAD Project (*.cad);;All Files (*)"));
        if (!file_path.isEmpty() && save_project_handler_) {
            save_project_handler_(file_path.toStdString());
        }
    });
    save_as_action->setShortcut(QKeySequence::SaveAs);
    
    QAction* checkpoints_act = file_menu->addAction(tr("Manage &Checkpoints..."));
    connect(checkpoints_act, &QAction::triggered, this, [this]() { showCheckpointsDialog(); });
    QAction* send_to_printer_action = file_menu->addAction(tr("Send to 3D &Printer..."));
    connect(send_to_printer_action, &QAction::triggered, this, [this]() {
        if (send_to_printer_handler_) {
            send_to_printer_handler_();
        } else if (statusBar()) {
            statusBar()->showMessage(tr("Send to printer: not available."), 2000);
        }
    });
    
    file_menu->addSeparator();
    
    recent_projects_menu_ = file_menu->addMenu(tr("Recent Projects"));
    // Recent projects will be populated dynamically
    
    file_menu->addSeparator();
    QAction* exit_action = file_menu->addAction(tr("E&xit"));
    connect(exit_action, &QAction::triggered, this, &QMainWindow::close);
    exit_action->setShortcut(QKeySequence::Quit);

    // Settings menu (language selection)
    QMenu* settings_menu = menu_bar->addMenu(tr("&Settings"));
    QMenu* language_menu = settings_menu->addMenu(tr("&Language"));

    QActionGroup* language_group = new QActionGroup(this);
    language_group->setExclusive(true);

    QSettings language_settings("HydraCAD", "HydraCAD");
    QString current_language = language_settings.value("ui/language", "en").toString();

    struct LanguageOption {
        QString code;
        QString label;
    };
    const std::vector<LanguageOption> languages = {
        {"en", tr("English")},
        {"de", tr("German")},
        {"zh", tr("Chinese")},
        {"ja", tr("Japanese")}
    };

    for (const auto& lang : languages) {
        QAction* action = language_menu->addAction(lang.label);
        action->setCheckable(true);
        action->setData(lang.code);
        if (lang.code == current_language) {
            action->setChecked(true);
        }
        language_group->addAction(action);
    }

    connect(language_group, &QActionGroup::triggered, this, [this](QAction* action) {
        if (!action) {
            return;
        }
        QSettings settings("HydraCAD", "HydraCAD");
        settings.setValue("ui/language", action->data().toString());
        QMessageBox::information(this, tr("Language"),
                                 tr("Language will be applied after restart."));
    });

    QAction* check_updates_action = settings_menu->addAction(tr("Check for &Updates..."));
    connect(check_updates_action, &QAction::triggered, this, [this]() {
        if (check_for_updates_handler_) {
            check_for_updates_handler_();
        } else if (statusBar()) {
            statusBar()->showMessage(tr("Check for updates: not available."), 2000);
        }
    });

    QAction* install_update_from_file_action = settings_menu->addAction(tr("Install &Update from File..."));
    connect(install_update_from_file_action, &QAction::triggered, this, [this]() {
        if (install_update_from_file_handler_) {
            install_update_from_file_handler_();
        } else if (statusBar()) {
            statusBar()->showMessage(tr("Install update from file: not available."), 2000);
        }
    });

    QAction* printers_dialog_action = settings_menu->addAction(tr("3D &Printers..."));
    connect(printers_dialog_action, &QAction::triggered, this, [this]() {
        if (printers_dialog_handler_) {
            printers_dialog_handler_();
        } else if (statusBar()) {
            statusBar()->showMessage(tr("3D Printers: not available."), 2000);
        }
    });

    QMenu* diagnostics_menu = settings_menu->addMenu(tr("&Diagnostics"));
    QAction* logs_act = diagnostics_menu->addAction(tr("Open &Logs Folder"));
    connect(logs_act, &QAction::triggered, this, [this]() {
        const QString logs_dir = logsDirectory();
        if (logs_dir.isEmpty()) {
            QMessageBox::warning(this, tr("Diagnostics"), tr("Logs folder is not available."));
            return;
        }
        QDesktopServices::openUrl(QUrl::fromLocalFile(logs_dir));
    });
    QAction* install_act = diagnostics_menu->addAction(tr("Open &Install Folder"));
    connect(install_act, &QAction::triggered, this, []() {
        QDesktopServices::openUrl(QUrl::fromLocalFile(QCoreApplication::applicationDirPath()));
    });
    diagnostics_menu->addSeparator();
    QAction* startup_log_act = diagnostics_menu->addAction(tr("Show &Startup Log"));
    connect(startup_log_act, &QAction::triggered, this, [this]() {
        const QString log_path = QDir(logsDirectory()).filePath("startup.log");
        const QString preview = readLogPreview(log_path);
        if (preview.isEmpty()) {
            QMessageBox::information(this, tr("Startup Log"), tr("No startup log found."));
            return;
        }
        QMessageBox::information(this, tr("Startup Log"), preview);
    });
    QAction* crash_log_act = diagnostics_menu->addAction(tr("Show &Last Crash Log"));
    connect(crash_log_act, &QAction::triggered, this, [this]() {
        const QString log_path = QDir(logsDirectory()).filePath("last_crash.log");
        const QString preview = readLogPreview(log_path);
        if (preview.isEmpty()) {
            QMessageBox::information(this, tr("Crash Log"), tr("No crash log found."));
            return;
        }
        QMessageBox::information(this, tr("Crash Log"), preview);
    });

    // Help menu (Inventor-style: Tutorials, Sample Projects)
    QMenu* help_menu = menu_bar->addMenu(tr("&Help"));
    QAction* get_started_action = help_menu->addAction(tr("&Get Started"));
    connect(get_started_action, &QAction::triggered, this, [this]() {
        QString doc_path = QCoreApplication::applicationDirPath() + "/../share/doc/hydracad/INSTALLATION.md";
        if (!QFile::exists(doc_path)) {
            doc_path = QCoreApplication::applicationDirPath() + "/../docs/INSTALLATION.md";
        }
        if (!QFile::exists(doc_path)) {
            QMessageBox::information(this, tr("Get Started"), tr("Documentation: see INSTALLATION.md in the project docs folder."));
            return;
        }
        QDesktopServices::openUrl(QUrl::fromLocalFile(doc_path));
    });
    QAction* tutorials_action = help_menu->addAction(tr("&Tutorials"));
    connect(tutorials_action, &QAction::triggered, this, [this]() {
        QString docs = QCoreApplication::applicationDirPath() + "/../docs";
        if (!QDir(docs).exists()) {
            docs = QCoreApplication::applicationDirPath() + "/../share/doc/hydracad";
        }
        if (QDir(docs).exists()) {
            QDesktopServices::openUrl(QUrl::fromLocalFile(docs));
        } else {
            QMessageBox::information(this, tr("Tutorials"), tr("Tutorials folder not found. See documentation in the project docs."));
        }
    });
    QAction* sample_projects_action = help_menu->addAction(tr("&Sample Projects"));
    connect(sample_projects_action, &QAction::triggered, this, [this]() {
        QString examples = QCoreApplication::applicationDirPath() + "/../examples";
        if (!QDir(examples).exists()) {
            examples = QCoreApplication::applicationDirPath() + "/../share/hydracad/examples";
        }
        if (QDir(examples).exists()) {
            QDesktopServices::openUrl(QUrl::fromLocalFile(examples));
        } else {
            QMessageBox::information(this, tr("Sample Projects"), tr("Sample projects folder not found. Create a project and save it to get started."));
        }
    });
    help_menu->addSeparator();
    QAction* about_action = help_menu->addAction(tr("&About Hydra CAD..."));
    connect(about_action, &QAction::triggered, this, [this]() {
        QMessageBox::about(this, tr("About Hydra CAD"),
                           tr("Hydra CAD – parametrisches 3D-CAD\n\n"
                              "Skizze → Feature → Verlauf → Zeichnung\n\n"
                              "© Hydra CAD Project"));
    });
    
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

void QtMainWindow::setParameterTable(const std::vector<std::string>& names, const std::vector<double>& values, const std::vector<std::string>& expressions) {
    QList<QStringList> rows;
    const size_t n = std::min(names.size(), std::min(values.size(), expressions.size()));
    for (size_t i = 0; i < n; ++i) {
        QStringList row;
        row << QString::fromStdString(names[i])
            << QString::number(values[i])
            << QString::fromStdString(expressions[i]);
        rows.append(row);
    }
    property_panel_->setParameterTable(rows);
}

void QtMainWindow::setIntegrationStatus(const std::string& status) {
    property_panel_->setIntegrationStatus(QString::fromStdString(status));
}

void QtMainWindow::setMateCount(int count) {
    property_panel_->setMateCount(count);
}

void QtMainWindow::setCommandHandler(const std::function<void(const std::string&)>& handler) {
    command_handler_ = handler;
    ribbon_->setCommandHandler([this](const QString& command) {
        if (command_handler_) {
            command_handler_(command.toStdString());
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
    browser_tree_->setCommandHandler([this](const QString& command) {
        if (command_handler_) command_handler_(command.toStdString());
        statusBar()->showMessage(tr("Command: %1").arg(command), 2000);
        log_panel_->appendLog(tr("Browser: %1").arg(command));
    });
    connect(browser_tree_, &QtBrowserTree::selectionChanged, this, [this](const QString& breadcrumb, const QString& nodeName) {
        if (!breadcrumb.isEmpty()) {
            property_panel_->setBreadcrumb(breadcrumb);
            statusBar()->showMessage(tr("Selected: %1").arg(nodeName), 2000);
        }
    });
}

void QtMainWindow::executeCommand(const std::string& command) {
    if (command_handler_) {
        command_handler_(command);
    }
}

void QtMainWindow::setAssemblySummary(const std::string& summary) {
    browser_tree_->setAssemblySummary(QString::fromStdString(summary));
}

void QtMainWindow::setMatesSummary(const std::string& summary) {
    browser_tree_->setMatesSummary(QString::fromStdString(summary));
}

void QtMainWindow::setReferenceGeometry(const QStringList& planeNames, const QStringList& axisNames, const QStringList& pointNames) {
    if (browser_tree_) {
        browser_tree_->setWorkPlanes(planeNames);
        browser_tree_->setWorkAxes(axisNames);
        browser_tree_->setWorkPoints(pointNames);
    }
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
    setWindowTitle(tr("Hydra CAD - %1").arg(QString::fromStdString(label.empty() ? "Untitled" : label)));
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

void QtMainWindow::setNewProjectHandler(const std::function<void()>& handler) {
    new_project_handler_ = handler;
}

void QtMainWindow::setSaveProjectHandler(const std::function<void(const std::string&)>& handler) {
    save_project_handler_ = handler;
}

void QtMainWindow::setLoadProjectHandler(const std::function<void(const std::string&)>& handler) {
    load_project_handler_ = handler;
}

void QtMainWindow::setCurrentProjectPath(const QString& path) {
    current_project_path_ = path;
}

void QtMainWindow::setCheckForUpdatesHandler(const std::function<void()>& handler) {
    check_for_updates_handler_ = handler;
}

void QtMainWindow::setInstallUpdateFromFileHandler(const std::function<void()>& handler) {
    install_update_from_file_handler_ = handler;
}

void QtMainWindow::setLicenseActivateHandler(const std::function<void()>& handler) {
    license_activate_handler_ = handler;
}

void QtMainWindow::setPrintersDialogHandler(const std::function<void()>& handler) {
    printers_dialog_handler_ = handler;
}

void QtMainWindow::setSendToPrinterHandler(const std::function<void()>& handler) {
    send_to_printer_handler_ = handler;
}

QString QtMainWindow::currentProjectPath() const {
    return current_project_path_;
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

void QtMainWindow::setProfileHandler(const std::function<void()>& handler) {
    profile_handler_ = handler;
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

void QtMainWindow::setImportFileHandler(const std::function<void(const std::string& path, const std::string& format)>& handler) {
    import_file_handler_ = handler;
}

void QtMainWindow::setExportFileHandler(const std::function<void(const std::string& path, const std::string& format)>& handler) {
    export_file_handler_ = handler;
}

static QString formatFromExtension(const QString& path) {
    const QString ext = QFileInfo(path).suffix().toLower();
    if (ext == QLatin1String("step") || ext == QLatin1String("stp")) return QStringLiteral("STEP");
    if (ext == QLatin1String("igs") || ext == QLatin1String("iges")) return QStringLiteral("IGES");
    if (ext == QLatin1String("stl")) return QStringLiteral("STL");
    if (ext == QLatin1String("obj")) return QStringLiteral("OBJ");
    if (ext == QLatin1String("dxf")) return QStringLiteral("DXF");
    if (ext == QLatin1String("dwg")) return QStringLiteral("DWG");
    if (ext == QLatin1String("3mf")) return QStringLiteral("3MF");
    if (ext == QLatin1String("gltf") || ext == QLatin1String("glb")) return QStringLiteral("GLTF");
    if (ext == QLatin1String("ply")) return QStringLiteral("PLY");
    return QStringLiteral("STEP");
}

void QtMainWindow::triggerImportDialog() {
    if (!import_file_handler_) {
        statusBar()->showMessage(tr("Import not available."), 3000);
        return;
    }
    const QString filter = tr("STEP (*.step *.stp);;IGES (*.igs *.iges);;STL (*.stl);;OBJ (*.obj);;DXF (*.dxf);;All Files (*)");
    QString path = QFileDialog::getOpenFileName(this, tr("Import Model"), current_project_path_.isEmpty() ? QString() : QFileInfo(current_project_path_).absolutePath(), filter);
    if (path.isEmpty()) return;
    const std::string pathStr = path.toStdString();
    const std::string formatStr = formatFromExtension(path).toStdString();
    import_file_handler_(pathStr, formatStr);
    statusBar()->showMessage(tr("Import: %1").arg(QString::fromStdString(pathStr)), 4000);
}

void QtMainWindow::triggerNewProject() {
    showNewProjectDialog();
}

void QtMainWindow::setNewProjectFromTemplateHandler(const std::function<void(const std::string&)>& handler) {
    new_project_from_template_handler_ = handler;
}

void QtMainWindow::setTemplateDirectory(const QString& path) {
    template_directory_ = path;
}

void QtMainWindow::showNewProjectDialog() {
    if (template_directory_.isEmpty()) {
        if (new_project_handler_) { new_project_handler_(); }
        else { setDocumentLabel("Untitled"); }
        return;
    }
    QDir dir(template_directory_);
    QStringList filters;
    filters << "*.cad" << "*.hcad";
    QStringList files = dir.entryList(filters, QDir::Files, QDir::Name);
    if (files.isEmpty()) {
        if (new_project_handler_) { new_project_handler_(); }
        else { setDocumentLabel("Untitled"); }
        return;
    }
    QDialog dlg(this);
    dlg.setWindowTitle(tr("New Project"));
    QVBoxLayout* layout = new QVBoxLayout(&dlg);
    QLabel* label = new QLabel(tr("Create from:"), &dlg);
    layout->addWidget(label);
    QListWidget* list = new QListWidget(&dlg);
    list->addItem(tr("Empty document"));
    list->item(0)->setData(Qt::UserRole, QString());
    for (const QString& name : files) {
        QString path = dir.absoluteFilePath(name);
        list->addItem(name);
        list->item(list->count() - 1)->setData(Qt::UserRole, path);
    }
    list->setCurrentRow(0);
    layout->addWidget(list);
    QDialogButtonBox* bbox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    connect(bbox, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(bbox, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    layout->addWidget(bbox);
    if (dlg.exec() != QDialog::Accepted) {
        return;
    }
    QListWidgetItem* item = list->currentItem();
    if (!item) {
        if (new_project_handler_) { new_project_handler_(); }
        return;
    }
    QString path = item->data(Qt::UserRole).toString();
    if (path.isEmpty()) {
        if (new_project_handler_) { new_project_handler_(); }
    } else if (new_project_from_template_handler_) {
        new_project_from_template_handler_(path.toStdString());
    } else {
        if (new_project_handler_) { new_project_handler_(); }
    }
}

void QtMainWindow::triggerOpenProject() {
    QString file_path = QFileDialog::getOpenFileName(this, tr("Open Project"), "", tr("CAD Project (*.cad);;All Files (*)"));
    if (!file_path.isEmpty() && load_project_handler_) {
        load_project_handler_(file_path.toStdString());
    }
}

void QtMainWindow::triggerSaveProject() {
    if (!current_project_path_.isEmpty() && save_project_handler_) {
        save_project_handler_(current_project_path_.toStdString());
        if (statusBar()) statusBar()->showMessage(tr("Project saved."), 3000);
    } else {
        QString file_path = QFileDialog::getSaveFileName(this, tr("Save Project"), "", tr("CAD Project (*.cad);;All Files (*)"));
        if (!file_path.isEmpty() && save_project_handler_) {
            save_project_handler_(file_path.toStdString());
        }
    }
}

void QtMainWindow::triggerExportDialog() {
    if (!export_file_handler_) {
        statusBar()->showMessage(tr("Export not available."), 3000);
        return;
    }
    const QString filter = tr("STEP (*.step *.stp);;IGES (*.igs *.iges);;STL (*.stl);;OBJ (*.obj);;DXF (*.dxf);;All Files (*)");
    QString path = QFileDialog::getSaveFileName(this, tr("Export Model"), current_project_path_.isEmpty() ? QString() : QFileInfo(current_project_path_).absolutePath(), filter);
    if (path.isEmpty()) return;
    const std::string pathStr = path.toStdString();
    const std::string formatStr = formatFromExtension(path).toStdString();
    export_file_handler_(pathStr, formatStr);
    statusBar()->showMessage(tr("Export: %1").arg(QString::fromStdString(pathStr)), 4000);
}

void QtMainWindow::setAskUnsavedChangesHandler(const std::function<UnsavedAction()>& handler) {
    ask_unsaved_handler_ = handler;
}

void QtMainWindow::setGetSavePathHandler(const std::function<std::string()>& handler) {
    get_save_path_handler_ = handler;
}

void QtMainWindow::setPropertyApplyHandler(const std::function<void()>& handler) {
    property_apply_handler_ = handler;
}

void QtMainWindow::setPropertyCancelHandler(const std::function<void()>& handler) {
    property_cancel_handler_ = handler;
}

void QtMainWindow::setPropertyApplyAndNewHandler(const std::function<void()>& handler) {
    property_apply_and_new_handler_ = handler;
}

QtMainWindow::UnsavedAction QtMainWindow::askUnsavedChanges() const {
    if (ask_unsaved_handler_) {
        return ask_unsaved_handler_();
    }
    return UnsavedAction::Discard;
}

std::string QtMainWindow::getSavePathForNewProject() const {
    if (get_save_path_handler_) {
        return get_save_path_handler_();
    }
    return {};
}

void QtMainWindow::setCheckpointsListProvider(const std::function<std::vector<std::string>(const std::string&)>& provider) {
    checkpoints_list_provider_ = provider;
}

void QtMainWindow::setLoadCheckpointHandler(const std::function<void(const std::string&)>& handler) {
    load_checkpoint_handler_ = handler;
}

void QtMainWindow::setDeleteCheckpointHandler(const std::function<void(const std::string&)>& handler) {
    delete_checkpoint_handler_ = handler;
}

void QtMainWindow::showCheckpointsDialog() {
    if (!checkpoints_list_provider_ || current_project_path_.isEmpty()) {
        QMessageBox::information(this, tr("Checkpoints"), tr("Open a project first, or no checkpoints available."));
        return;
    }
    std::vector<std::string> list = checkpoints_list_provider_(current_project_path_.toStdString());
    QDialog dlg(this);
    dlg.setWindowTitle(tr("Manage Checkpoints"));
    QVBoxLayout* layout = new QVBoxLayout(&dlg);
    QListWidget* listWidget = new QListWidget(&dlg);
    for (const auto& path : list) {
        QFileInfo fi(QString::fromStdString(path));
        listWidget->addItem(fi.fileName() + QLatin1String(" (") + fi.absolutePath() + QLatin1String(")"));
        listWidget->item(listWidget->count() - 1)->setData(Qt::UserRole, QString::fromStdString(path));
    }
    if (list.empty()) {
        listWidget->addItem(tr("No checkpoints found for this project."));
        listWidget->item(0)->setFlags(listWidget->item(0)->flags() & ~Qt::ItemIsEnabled);
    }
    layout->addWidget(listWidget);
    QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Close, &dlg);
    if (!list.empty()) {
        QPushButton* openBtn = buttons->addButton(tr("Open"), QDialogButtonBox::AcceptRole);
        QPushButton* deleteBtn = buttons->addButton(tr("Delete"), QDialogButtonBox::DestructiveRole);
        connect(openBtn, &QPushButton::clicked, &dlg, [&dlg, listWidget, this]() {
            QListWidgetItem* item = listWidget->currentItem();
            if (item && item->data(Qt::UserRole).isValid() && !item->data(Qt::UserRole).toString().isEmpty() && load_checkpoint_handler_) {
                load_checkpoint_handler_(item->data(Qt::UserRole).toString().toStdString());
                dlg.accept();
            }
        });
        connect(deleteBtn, &QPushButton::clicked, &dlg, [&dlg, listWidget, this]() {
            QListWidgetItem* item = listWidget->currentItem();
            if (item && item->data(Qt::UserRole).isValid() && !item->data(Qt::UserRole).toString().isEmpty() && delete_checkpoint_handler_) {
                QString path = item->data(Qt::UserRole).toString();
                delete_checkpoint_handler_(path.toStdString());
                delete listWidget->takeItem(listWidget->row(item));
            }
        });
    }
    connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    layout->addWidget(buttons);
    dlg.exec();
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
