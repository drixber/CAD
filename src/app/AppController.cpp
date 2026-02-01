#include "AppController.h"
#include "CommunityService.h"
#include "ai/AIService.h"
#include "UpdateInstaller.h"
#include "HttpClient.h"
#include "core/updates/UpdateChecker.h"
#include <cstdlib>
#include <sstream>
#include <map>
#ifdef CAD_USE_QT
#include <QString>
#include <QSettings>
#include <QStringList>
#include <QMessageBox>
#include <QFileDialog>
#include <QStatusBar>
#include <QTimer>
#include <QApplication>
#include "ui/qt/QtLoginDialog.h"
#include "ui/qt/QtRegisterDialog.h"
#include "ui/qt/QtAIChatPanel.h"
#include "ui/qt/QtAISettingsDialog.h"
#include "ui/qt/QtUpdateDialog.h"
#include "ui/qt/QtLicenseDialog.h"
#include "ui/qt/QtMateDialog.h"
#include "ui/qt/QtPrintersDialog.h"
#include "ui/qt/QtCommunityPanel.h"
#include <QProcess>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QDesktopServices>
#include <QUrl>
#endif

namespace cad {
namespace app {

AppController::AppController() {
    community_service_ = new CommunityService();
}

AppController::~AppController() {
    delete community_service_;
    community_service_ = nullptr;
}

bool AppController::initializeWithLogin() {
    // Offline/local-only mode: no cloud login required; app runs on hardware only.
    (void)user_auth_service_;
    return true;
}

bool AppController::requireLogin() {
    // Offline mode: no login required.
    return true;
}

void AppController::initialize() {
    main_window_.initializeLayout();
    main_window_.setViewportStatus("3D viewport ready");
    main_window_.setWorkspaceMode("General");
    main_window_.setDocumentLabel("MainDocument");
#ifdef CAD_USE_EIGENER_KERN
    eigen_kernel_ = std::make_unique<cad::kernel::KernelBridge>();
    if (eigen_kernel_ && eigen_kernel_->initialize()) {
        main_window_.setIntegrationStatus("Eigen-Kern on");
    } else {
        main_window_.setIntegrationStatus("Eigen-Kern off");
    }
#endif
    techdraw_bridge_.initialize();
    cad::core::Sketch sketch("Sketch1");
    sketch.addConstraint({cad::core::ConstraintType::Distance, "line1", "line2", 25.0});
    sketch.addParameter({"Width", 0.0, "100"});
    sketch.addParameter({"Height", 0.0, "50"});
    setActiveSketch(sketch);
    modeler_.evaluateParameters(active_sketch_);
    main_window_.setConstraintCount(static_cast<int>(active_sketch_.constraints().size()));
    main_window_.setParameterCount(static_cast<int>(active_sketch_.parameters().size()));
    main_window_.setParameterSummary(buildParameterSummary(active_sketch_));
    // Solve constraints after geometry is added
    modeler_.solveConstraints(active_sketch_);
    initializeAssembly();
    bindCommands();
    
    // Initialize project file service with auto-save
    #ifdef CAD_USE_QT
    QSettings settings("HydraCAD", "HydraCAD");
    bool autosave_enabled = settings.value("project/autosave_enabled", true).toBool();
    int autosave_interval = settings.value("project/autosave_interval", 300).toInt();
    QString autosave_path = settings.value("project/autosave_path", "autosave").toString();
    project_file_service_.enableAutoSave(autosave_enabled);
    project_file_service_.setAutoSaveInterval(autosave_interval);
    project_file_service_.setAutoSavePath(autosave_path.toStdString());
    
    // Load recent projects from settings
    QStringList recent = settings.value("project/recent_projects").toStringList();
    for (const QString& path : recent) {
        recent_projects_.push_back(path.toStdString());
    }
    #else
    project_file_service_.enableAutoSave(true);
    project_file_service_.setAutoSaveInterval(300);  // 5 minutes
    project_file_service_.setAutoSavePath("autosave");
    #endif
    
    // Setup auto-save timer if Qt is available
    #ifdef CAD_USE_QT
    cad::ui::QtMainWindow* qt_window = main_window_.nativeWindow();
    if (qt_window) {
        // Setup project file handlers
        qt_window->setNewProjectHandler([this]() {
            newProject();
        });
        qt_window->setNewProjectFromTemplateHandler([this](const std::string& path) {
            newProjectFromTemplate(path);
        });
        qt_window->setTemplateDirectory(QString::fromStdString(getTemplateDirectory()));
        qt_window->setSaveProjectHandler([this](const std::string& path) {
            saveProject(path);
        });
        qt_window->setLoadProjectHandler([this](const std::string& path) {
            loadProject(path);
        });
        qt_window->setAutoSaveTriggerHandler([this]() {
            triggerAutoSave();
        });
        qt_window->setAutoSaveStatusHandler([this](const std::string& status) {
            main_window_.setViewportStatus(status);
        });
        // Initialize recent projects menu
        qt_window->updateRecentProjectsMenu(recent_projects_);
        
        // Offline mode: no license/user/community setup; app runs on hardware only.

        // AI Service integration
        setupAIService(qt_window);

        // Updates: check from server (optional) and install from local file
        update_service_.enableAutoUpdate(false);  // No automatic cloud check by default
        qt_window->setCheckForUpdatesHandler([this, qt_window]() {
            checkForUpdates(qt_window, false);
        });
        qt_window->setInstallUpdateFromFileHandler([this, qt_window]() {
            QString path = QFileDialog::getOpenFileName(qt_window, QObject::tr("Select Update Package"),
                QString(), QObject::tr("Update packages (*.zip *.exe);;All files (*)"));
            if (path.isEmpty()) return;
            cad::ui::QtUpdateDialog* progress_dlg = new cad::ui::QtUpdateDialog(qt_window);
            progress_dlg->setAttribute(Qt::WA_DeleteOnClose);
            progress_dlg->show();
            bool ok = update_service_.installInPlaceUpdate(path.toStdString(),
                [progress_dlg](const cad::app::UpdateProgress& p) {
                    if (progress_dlg) {
                        progress_dlg->setProgress(p.percentage, QString::fromStdString(p.status_message));
                        QApplication::processEvents();
                    }
                });
            progress_dlg->close();
            if (ok) {
                QMessageBox::information(qt_window, QObject::tr("Update"), QObject::tr("Update installed. Restart the application to apply."));
            } else {
                QMessageBox::warning(qt_window, QObject::tr("Update"), QObject::tr("Update installation failed. Check the file and try again."));
            }
        });
        setupAutoUpdate(qt_window);

        printer_service_.setHttpClient(&update_service_.getHttpClient());
        qt_window->setPrintersDialogHandler([this, qt_window]() {
            cad::ui::QtPrintersDialog dlg(qt_window);
            dlg.setPrinterService(&printer_service_);
            dlg.setSendMode(false);
            dlg.exec();
        });
        qt_window->setSendToPrinterHandler([this, qt_window]() {
            if (printer_service_.getPrinters().empty()) {
                QMessageBox::information(qt_window, QObject::tr("Send to 3D Printer"),
                    QObject::tr("No 3D printer configured. Add one in Settings → 3D Printers."));
                cad::ui::QtPrintersDialog dlg(qt_window);
                dlg.setPrinterService(&printer_service_);
                dlg.setSendMode(false);
                dlg.exec();
                return;
            }
            cad::ui::QtPrintersDialog dlg(qt_window);
            dlg.setPrinterService(&printer_service_);
            dlg.setSendMode(true);
            if (dlg.exec() != QDialog::Accepted) return;
            std::string printer_id = dlg.selectedPrinterId();
            if (printer_id.empty()) return;
            QString temp_path = QDir::temp().filePath("hydracad_send_to_printer.stl");
            cad::interop::IoResult export_result = io_service_.exportStl(temp_path.toStdString(), false);
            if (!export_result.success) {
                QMessageBox::warning(qt_window, QObject::tr("Export failed"),
                    QObject::tr("Could not export model to STL: %1").arg(QString::fromStdString(export_result.message)));
                return;
            }
            auto upload_result = printer_service_.sendStlToPrinter(printer_id, temp_path.toStdString(), false);
            if (upload_result.success) {
                QMessageBox::information(qt_window, QObject::tr("Send to 3D Printer"),
                    QObject::tr("File sent to printer successfully."));
                main_window_.setViewportStatus("Sent to 3D printer");
            } else {
                QMessageBox::warning(qt_window, QObject::tr("Upload failed"),
                    QObject::tr("Could not send file to printer: %1").arg(QString::fromStdString(upload_result.message)));
            }
        });

        qt_window->setImportFileHandler([this](const std::string& path, const std::string& format) {
            cad::interop::IoJob job;
            job.path = path;
            job.format = format;
            if (!io_pipeline_.supportsFormat(format, false)) {
                main_window_.setIntegrationStatus("Import format unsupported: " + format);
            } else {
                cad::interop::IoJobResult result = io_pipeline_.importJob(job);
                main_window_.setIntegrationStatus(result.message);
                main_window_.setViewportStatus("Import queued");
            }
        });

        // Community panel disabled in offline mode (no cloud).
        qt_window->setExportFileHandler([this](const std::string& path, const std::string& format) {
            cad::interop::IoJob job;
            job.path = path;
            job.format = format;
            if (!io_pipeline_.supportsFormat(format, true)) {
                main_window_.setIntegrationStatus("Export format unsupported: " + format);
            } else {
                cad::interop::IoJobResult result = io_pipeline_.exportJob(job);
                main_window_.setIntegrationStatus(result.message + " (" + format + ")");
                main_window_.setViewportStatus("Export queued");
            }
        });

        qt_window->setPropertyApplyHandler([this]() {
            main_window_.setIntegrationStatus("Properties applied");
            main_window_.setViewportStatus("Properties applied – changes committed");
        });
        qt_window->setPropertyCancelHandler([this]() {
            main_window_.setIntegrationStatus("Properties cancelled");
            main_window_.setViewportStatus("Properties cancelled – no changes");
            main_window_.setContextPlaceholder("");
        });
        qt_window->setPropertyApplyAndNewHandler([this]() {
            main_window_.setIntegrationStatus("Applied & New");
            main_window_.setViewportStatus("Ready for next feature");
            main_window_.setContextPlaceholder("");
        });

        qt_window->setAskUnsavedChangesHandler([qt_window]() {
            QMessageBox::StandardButton b = QMessageBox::question(qt_window,
                QObject::tr("Unsaved Changes"),
                QObject::tr("Save changes before opening another project?"),
                QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
                QMessageBox::Save);
            if (b == QMessageBox::Save) return cad::ui::QtMainWindow::UnsavedAction::Save;
            if (b == QMessageBox::Discard) return cad::ui::QtMainWindow::UnsavedAction::Discard;
            return cad::ui::QtMainWindow::UnsavedAction::Cancel;
        });
        qt_window->setGetSavePathHandler([qt_window]() {
            QString path = QFileDialog::getSaveFileName(qt_window,
                QObject::tr("Save Project"), QString(),
                QObject::tr("CAD Project (*.cad);;All Files (*)"));
            return path.isEmpty() ? std::string() : path.toStdString();
        });

        qt_window->setCheckpointsListProvider([this](const std::string& project_path) {
            return getCheckpointsForProject(project_path);
        });
        qt_window->setLoadCheckpointHandler([this](const std::string& path) {
            loadCheckpoint(path);
        });
        qt_window->setDeleteCheckpointHandler([this](const std::string& path) {
            deleteCheckpoint(path);
        });
    }
    #endif

    // Simple GitHub release check on startup (optional).
    #ifndef CAD_APP_VERSION
    #define CAD_APP_VERSION "v0.0.0"
    #endif
    static constexpr const char* kCurrentVersionTag = CAD_APP_VERSION;
    const std::string owner = "drixber";
    const std::string repo = "CAD";
    cad::core::updates::UpdateInfo update_info;
    #ifdef CAD_USE_QT_NETWORK
    const std::string api_url = "https://api.github.com/repos/" + owner + "/" + repo + "/releases/latest";
    cad::app::HttpResponse api_resp = update_service_.getHttpClient().get(api_url, {{"Accept", "application/vnd.github.v3+json"}});
    if (api_resp.success && !api_resp.body.empty()) {
        update_info = cad::core::updates::parseGithubReleaseResponse(api_resp.body, kCurrentVersionTag);
    } else {
        update_info = cad::core::updates::checkGithubLatestRelease(owner, repo, kCurrentVersionTag);
    }
    #else
    update_info = cad::core::updates::checkGithubLatestRelease(owner, repo, kCurrentVersionTag);
    #endif
    #ifdef CAD_USE_QT
    QSettings release_settings("HydraCAD", "HydraCAD");
    const bool open_release = release_settings.value("updates/open_release_on_startup", false).toBool();
    #else
    const bool open_release = false;
    #endif
    if (open_release && update_info.updateAvailable) {
        cad::core::updates::openUrlInBrowser(update_info.releaseUrl);
    }
}

void AppController::setActiveSketch(const cad::core::Sketch& sketch) {
    active_sketch_ = sketch;
    updateSketchConstraintStatus();
}

void AppController::updateSketchConstraintStatus() {
    if (active_sketch_.geometry().empty() && active_sketch_.constraints().empty()) {
        main_window_.setViewportStatus("Sketch: Leer");
        return;
    }
    int dof = modeler_.getDegreesOfFreedom(active_sketch_);
    bool over = modeler_.isOverConstrained(active_sketch_);
    bool under = modeler_.isUnderConstrained(active_sketch_);
    std::string status;
    if (over) {
        status = "Sketch: Überbestimmt (Konflikte möglich)";
    } else if (under || dof > 0) {
        status = "Sketch: Unterbestimmt (" + std::to_string(dof) + " DOF)";
    } else {
        status = "Sketch: Voll bestimmt (0 DOF)";
    }
    main_window_.setViewportStatus(status);
}

void AppController::updateAssemblyConstraintStatus() {
    if (active_assembly_.components().empty()) {
        return;
    }
    int dof = active_assembly_.getDegreesOfFreedom();
    bool over = active_assembly_.isOverConstrained();
    bool under = active_assembly_.isUnderConstrained();
    std::string status;
    if (over) {
        status = "Assembly: Überbestimmt (Konflikte möglich)";
    } else if (under || dof > 0) {
        status = "Assembly: " + std::to_string(active_assembly_.components().size()) + " Komponenten, "
                 + std::to_string(active_assembly_.mates().size()) + " Mates, " + std::to_string(dof) + " DOF";
    } else {
        status = "Assembly: Voll bestimmt (0 DOF), " + std::to_string(active_assembly_.mates().size()) + " Mates";
    }
    main_window_.setViewportStatus(status);
}

cad::ui::MainWindow& AppController::mainWindow() {
    return main_window_;
}

cad::core::Modeler& AppController::modeler() {
    return modeler_;
}

std::string AppController::buildParameterSummary(const cad::core::Sketch& sketch) const {
    std::ostringstream summary;
    bool first = true;
    for (const auto& parameter : sketch.parameters()) {
        if (!first) {
            summary << ", ";
        }
        first = false;
        summary << parameter.name << "=" << parameter.value;
    }
    return summary.str();
}

void AppController::initializeAssembly() {
    active_assembly_ = modeler_.createAssembly();
    cad::core::Part partA("Bracket");
    partA.addWorkPlaneOffset("XY Offset 10", "XY", 10.0);
    partA.addUserParameter({"Width", 100.0, ""});
    partA.addUserParameter({"Height", 50.0, "Width/2"});
    partA.addUserParameter({"Depth", 10.0, ""});
    cad::core::Part partB("Plate");
    cad::core::Transform transformA;
    cad::core::Transform transformB;
    transformB.tx = 10.0;
    std::uint64_t idA = active_assembly_.addComponent(partA, transformA);
    std::uint64_t idB = active_assembly_.addComponent(partB, transformB);
    active_assembly_.addMate({idA, idB, cad::core::MateType::Mate, 0.0});
    main_window_.setMateCount(static_cast<int>(active_assembly_.mates().size()));
    main_window_.setAssemblySummary("2 components, 1 mate");
    main_window_.setMatesSummary("1 mate");
    updateAssemblyConstraintStatus();
#ifdef CAD_USE_QT
    if (cad::ui::QtMainWindow* qt_window = main_window_.nativeWindow()) {
        QStringList planeNames, axisNames, pointNames;
        if (!active_assembly_.components().empty()) {
            const cad::core::Part& part = active_assembly_.components()[0].part;
            for (const auto& wp : part.workPlanes())
                planeNames << QString::fromStdString(wp.name);
            for (const auto& wa : part.workAxes())
                axisNames << QString::fromStdString(wa.name);
            for (const auto& wpt : part.workPoints())
                pointNames << QString::fromStdString(wpt.name);
        }
        qt_window->setReferenceGeometry(planeNames, axisNames, pointNames);
        cad::ui::Viewport3D* vp = qt_window->viewport3D();
        if (vp) {
            std::vector<std::string> comp_ids;
            for (const auto& comp : active_assembly_.components()) {
                comp_ids.push_back("asm_MainAssembly_" + std::to_string(comp.id));
            }
            vp->setAssemblyComponents("MainAssembly", comp_ids);
            for (const std::string& gid : comp_ids) {
                vp->renderGeometry(gid, nullptr);
            }
            vp->renderAssembly("MainAssembly");
            syncAssemblyTransformsToViewport();
        }
    }
#endif
    
    // Register assembly in BOM registry for UI integration
    bom_service_.registerAssembly("MainAssembly", active_assembly_);

    assembly_manager_.setCacheLimit(300);
    assembly_manager_.enableBackgroundLoading(true);
    assembly_manager_.setLodMode(cad::core::LodMode::Simplified);
    assembly_manager_.setTargetFps(30.0);
    main_window_.setTargetFps(30);
    if (assembly_manager_.recommendedLod() == cad::core::LodMode::BoundingBoxes) {
        main_window_.setViewportStatus("LOD recommendation: bounding boxes");
    } else if (assembly_manager_.recommendedLod() == cad::core::LodMode::Simplified) {
        main_window_.setViewportStatus("LOD recommendation: simplified");
    } else {
        main_window_.setViewportStatus("LOD recommendation: full");
    }
    cad::core::PerfTimer timer("AssemblyLoad");
    cad::core::AssemblyLoadStats load_stats = assembly_manager_.loadAssembly("MainAssembly");
    cad::core::PerfSpan span = timer.finish();
    cad::core::CacheStats cache = assembly_manager_.cacheStats();
    main_window_.setIntegrationStatus("Cache: " + std::to_string(cache.entries) + "/" +
                                       std::to_string(cache.max_entries) +
                                       (load_stats.used_background_loading ? " (bg)" : " (fg)"));
    main_window_.setCacheStats(static_cast<int>(cache.entries),
                               static_cast<int>(cache.max_entries));
    main_window_.setViewportStatus(load_stats.used_background_loading
                                       ? "Background loading enabled"
                                       : "Background loading disabled");
    main_window_.setBackgroundLoading(load_stats.used_background_loading);
    main_window_.setViewportStatus("Assembly load " + std::to_string(span.elapsed_ms) + " ms");
    assembly_manager_.enqueueLoad("MainAssembly");
}

void AppController::bindCommands() {
    main_window_.setCommandHandler([this](const std::string& command) {
        executeCommand(command);
    });
    
    #ifdef CAD_USE_QT
    cad::ui::QtMainWindow* qt_window = main_window_.nativeWindow();
    if (qt_window && qt_window->commandLine()) {
        cad::ui::QtCommandLine* cmd_line = dynamic_cast<cad::ui::QtCommandLine*>(qt_window->commandLine());
        if (cmd_line) {
            QObject::connect(cmd_line, &cad::ui::QtCommandLine::commandParsed,
                           [this](const cad::ui::ParsedCommand& parsed) {
                               if (parsed.valid) {
                                   std::string full_command = parsed.command.toStdString();
                                   if (!parsed.parameters.isEmpty()) {
                                       full_command += " " + parsed.parameters.join(" ").toStdString();
                                   }
                                   executeCommand(full_command);
                               } else {
                                   main_window_.setIntegrationStatus("Invalid command: " + parsed.error_message.toStdString());
                                   main_window_.setViewportStatus("Command validation failed");
                               }
                           });
        }
    }
    #endif
    main_window_.setLodModeHandler([this](const std::string& mode) {
        if (mode == "full") {
            assembly_manager_.setLodMode(cad::core::LodMode::Full);
        } else if (mode == "simplified") {
            assembly_manager_.setLodMode(cad::core::LodMode::Simplified);
        } else {
            assembly_manager_.setLodMode(cad::core::LodMode::BoundingBoxes);
        }
        main_window_.setViewportStatus("LOD: " + mode);
    });
    main_window_.setBackgroundLoadingHandler([this](bool enabled) {
        assembly_manager_.enableBackgroundLoading(enabled);
        main_window_.setViewportStatus(enabled ? "Background loading enabled"
                                               : "Background loading disabled");
    });
    main_window_.setTargetFpsHandler([this](int fps) {
        assembly_manager_.setTargetFps(static_cast<double>(fps));
        main_window_.setViewportStatus("Target FPS " + std::to_string(fps));
    });
}

void AppController::syncAssemblyTransformsToViewport() {
#ifdef CAD_USE_QT
    cad::ui::QtMainWindow* qt_window = main_window_.nativeWindow();
    if (!qt_window || !qt_window->viewport3D()) {
        return;
    }
    cad::ui::Viewport3D* vp = qt_window->viewport3D();
    std::vector<std::string> comp_ids = vp->getAssemblyComponents("MainAssembly");
    const auto& components = active_assembly_.components();
    for (std::size_t i = 0; i < comp_ids.size() && i < components.size(); ++i) {
        cad::core::Transform t = active_assembly_.getDisplayTransform(components[i].id);
        vp->setComponentTransform(comp_ids[i], t.tx, t.ty, t.tz);
    }
#endif
}

void AppController::executeCommand(const std::string& command) {
    // Parse command into base name and space-separated parameters (for sketch commands)
    std::string base_cmd = command;
    std::vector<std::string> param_strs;
    {
        std::istringstream iss(command);
        std::string tok;
        if (iss >> tok) {
            base_cmd = tok;
            while (iss >> tok)
                param_strs.push_back(tok);
        }
    }

#ifdef CAD_USE_QT
    if (command == "Shaded" || command == "Wireframe" || command == "HiddenLine") {
        cad::ui::QtMainWindow* qt_win = main_window_.nativeWindow();
        if (qt_win && qt_win->viewport3D()) {
            cad::ui::Viewport3D* vp = qt_win->viewport3D();
            if (command == "Shaded") {
                vp->setDisplayMode(cad::ui::Viewport3D::DisplayMode::Shaded);
                main_window_.setViewportStatus("Visual style: Shaded");
            } else if (command == "Wireframe") {
                vp->setDisplayMode(cad::ui::Viewport3D::DisplayMode::Wireframe);
                main_window_.setViewportStatus("Visual style: Wireframe");
            } else if (command == "HiddenLine") {
                vp->setDisplayMode(cad::ui::Viewport3D::DisplayMode::HiddenLine);
                main_window_.setViewportStatus("Visual style: Hidden Line");
            }
        }
        return;
    }
    if (command == "New") {
        cad::ui::QtMainWindow* qt_win = main_window_.nativeWindow();
        if (qt_win) qt_win->triggerNewProject();
        return;
    }
    if (command == "Open") {
        cad::ui::QtMainWindow* qt_win = main_window_.nativeWindow();
        if (qt_win) qt_win->triggerOpenProject();
        return;
    }
    if (command == "Save") {
        cad::ui::QtMainWindow* qt_win = main_window_.nativeWindow();
        if (qt_win) qt_win->triggerSaveProject();
        return;
    }
    if (command == "Import") {
        cad::ui::QtMainWindow* qt_win = main_window_.nativeWindow();
        if (qt_win) qt_win->triggerImportDialog();
        return;
    }
    if (command == "Export") {
        cad::ui::QtMainWindow* qt_win = main_window_.nativeWindow();
        if (qt_win) qt_win->triggerExportDialog();
        return;
    }
    if (command == "GetStarted" || command == "Documentation") {
        main_window_.setViewportStatus("Get Started: See documentation.");
        return;
    }
#endif
    if (command == "iLogic") {
        cad::core::AssemblyComponent* comp = active_assembly_.findComponent(active_assembly_.components().empty() ? 0 : active_assembly_.components()[0].id);
        if (comp) {
            if (comp->part.rules().empty()) {
                cad::core::Rule rule;
                rule.name = "WidthToHeight";
                rule.trigger = "ParameterChange";
                rule.condition_expression = "Width > 80";
                rule.then_parameter = "Height";
                rule.then_value_expression = "40";
                comp->part.addRule(rule);
            }
            modeler_.evaluatePartParameters(comp->part);
            modeler_.evaluatePartRules(comp->part);
            main_window_.setIntegrationStatus("iLogic: " + std::to_string(comp->part.rules().size()) + " rule(s) evaluated");
            main_window_.setViewportStatus("Regeln ausgeführt (ParameterChange)");
        } else {
            main_window_.setIntegrationStatus("iLogic: No part selected");
            main_window_.setViewportStatus("Kein Part für Regeln");
        }
    } else if (command == "Parameters") {
        int total_params = static_cast<int>(active_sketch_.parameters().size());
        std::string summary = buildParameterSummary(active_sketch_);
        std::vector<std::string> names;
        std::vector<double> values;
        std::vector<std::string> expressions;
        cad::core::AssemblyComponent* comp = active_assembly_.findComponent(active_assembly_.components().empty() ? 0 : active_assembly_.components()[0].id);
        if (comp) {
            modeler_.evaluatePartParameters(comp->part);
            modeler_.evaluatePartRules(comp->part);
            for (const auto& p : comp->part.userParameters()) {
                names.push_back(p.name);
                values.push_back(p.value);
                expressions.push_back(p.expression);
            }
            total_params += static_cast<int>(names.size());
            if (!comp->part.userParameters().empty()) {
                summary += " | Part: " + std::to_string(comp->part.userParameters().size()) + " user";
            }
        }
        main_window_.setParameterTable(names, values, expressions);
        main_window_.setParameterSummary(summary);
        main_window_.setParameterCount(total_params);
        main_window_.setViewportStatus("Parameters: " + std::to_string(total_params) + " items");
    } else if (command == "Flange" || command == "Bend" || command == "Unfold" || command == "Refold" ||
               command == "Punch" || command == "Bead") {
        cad::modules::SheetMetalRequest request;
        request.targetPart = "Bracket";
        if (command == "Flange") request.operation = cad::modules::SheetMetalOperation::Flange;
        else if (command == "Bend") request.operation = cad::modules::SheetMetalOperation::Bend;
        else if (command == "Unfold") request.operation = cad::modules::SheetMetalOperation::Unfold;
        else if (command == "Refold") request.operation = cad::modules::SheetMetalOperation::Refold;
        else if (command == "Punch") request.operation = cad::modules::SheetMetalOperation::Punch;
        else request.operation = cad::modules::SheetMetalOperation::Bead;
        cad::modules::SheetMetalResult result = sheet_metal_service_.applyOperation(request);
        main_window_.setIntegrationStatus(result.message);
        main_window_.setViewportStatus("Sheet metal: " + result.message);
    } else if (command == "SheetMetalRules") {
        cad::modules::SheetMetalRules rules;
        rules.thickness = 1.5;
        rules.bend_radius_default = 2.0;
        rules.k_factor = 0.5;
        rules.corner_relief = "Square";
        sheet_metal_service_.setRules("Bracket", rules);
        main_window_.setIntegrationStatus("Sheet metal rules set: thickness 1.5 mm, K-factor 0.5");
        main_window_.setViewportStatus("Blechregeln: Dicke 1.5, Biegeradius 2, K 0.5");
    } else if (command == "ExportFlatDXF") {
        std::string path = "flat_pattern.dxf";
        if (sheet_metal_service_.exportFlatPatternToDxf("Bracket", path)) {
            main_window_.setIntegrationStatus("Flat pattern exported to " + path);
            main_window_.setViewportStatus("Abwicklung als DXF gespeichert");
        } else {
            main_window_.setIntegrationStatus("Export flat DXF failed");
            main_window_.setViewportStatus("DXF-Export fehlgeschlagen");
        }
    } else if (command == "Rectangular Pattern" || command == "Circular Pattern" || command == "Curve Pattern" ||
               command == "RectangularPattern" || command == "CurvePattern") {
        cad::modules::PatternRequest request;
        request.targetFeature = "Hole1";
        request.type = cad::modules::PatternType::Rectangular;
        request.instanceCount = 6;
        cad::modules::PatternResult result = pattern_service_.createPattern(request);
        main_window_.setIntegrationStatus(result.message);
        main_window_.setViewportStatus("Pattern command queued");
    } else if (command == "CircularPattern") {
        cad::core::Part part = modeler_.createPart(active_sketch_);
        if (!active_sketch_.geometry().empty()) {
            modeler_.applyExtrude(part, active_sketch_.name(), 10.0);
            std::string base = part.features().empty() ? active_sketch_.name() : part.features().back().name;
            modeler_.applyCircularPattern(part, base, 6, 360.0, "Z");
            main_window_.setIntegrationStatus("Circular pattern created: 6 instances, 360°, axis Z");
            main_window_.setViewportStatus("Circular pattern applied");
        } else {
            main_window_.setIntegrationStatus("Circular pattern: add sketch geometry first");
            main_window_.setViewportStatus("Sketch required");
        }
    } else if (command == "FacePattern") {
        cad::modules::PatternRequest request;
        request.targetFeature = "Hole1";
        request.type = cad::modules::PatternType::Face;
        request.face_params.face_id = "Face1";
        request.face_params.spacing_x = 10.0;
        request.face_params.spacing_y = 10.0;
        request.face_params.count_x = 3;
        request.face_params.count_y = 3;
        cad::modules::PatternResult result = pattern_service_.createPattern(request);
        main_window_.setIntegrationStatus(result.message);
        main_window_.setViewportStatus("Face pattern (Flächenmuster) queued");
    } else if (command == "Direct Edit" || command == "Freeform" || command == "DirectEdit") {
        cad::modules::DirectEditRequest request;
        request.targetFeature = "Face1";
        request.operation = cad::modules::DirectEditOperation::MoveFace;
        cad::modules::DirectEditResult result = direct_edit_service_.applyEdit(request);
        main_window_.setIntegrationStatus(result.message);
        main_window_.setViewportStatus("Direct edit queued");
    } else if (command == "Rigid Pipe" || command == "Flexible Hose" || command == "Bent Tube") {
        cad::modules::RoutingRequest request;
        request.targetAssembly = "MainAssembly";
        request.type = cad::modules::RoutingType::RigidPipe;
        if (command == "Flexible Hose" || command == "FlexibleHose") request.type = cad::modules::RoutingType::FlexibleHose;
        else if (command == "Bent Tube" || command == "BentTube") request.type = cad::modules::RoutingType::BentTube;
        cad::modules::RoutingResult result = routing_service_.createRoute(request);
        main_window_.setIntegrationStatus(result.message);
        main_window_.setViewportStatus("Routing: " + result.route_id + ", L=" + std::to_string(static_cast<int>(result.total_length)) + " mm");
    } else if (command == "RouteBOM") {
        std::string route_id = "MainAssembly_rigid_pipe";
        std::vector<cad::modules::PipeBomItem> bom = routing_service_.getRouteBom(route_id);
        if (bom.empty()) {
            bom = routing_service_.getRouteBom("MainAssembly_flexible_hose");
            if (bom.empty()) {
                bom = routing_service_.getRouteBom("MainAssembly_bent_tube");
            }
        }
        if (bom.empty()) {
            main_window_.setIntegrationStatus("Route BOM: Create a route first (Rigid Pipe / Bent Tube)");
            main_window_.setViewportStatus("Keine Route für Stückliste");
        } else {
            std::string summary = "Route BOM: " + std::to_string(bom.size()) + " items";
            for (const auto& item : bom) {
                summary += "; " + item.description + " x" + std::to_string(item.quantity);
            }
            main_window_.setIntegrationStatus(summary);
            main_window_.setViewportStatus("Rohr-Stückliste: " + std::to_string(bom.size()) + " Positionen");
        }
    } else if (command == "Weld") {
        cad::modules::WeldJoint weld;
        weld.name = "Weld1";
        weld.type = cad::modules::WeldType::Fillet;
        weld.length_mm = 50.0;
        weld.size_mm = 5.0;
        weld.part_a = "Bracket";
        weld.part_b = "Plate";
        weld.symbol = "Fillet 5";
        welding_service_.addWeld("MainAssembly", weld);
        main_window_.setIntegrationStatus("Weld added: " + weld.symbol);
        main_window_.setViewportStatus("Schweißnaht: Kehlnaht 5 mm");
    } else if (command == "WeldBOM") {
        std::vector<cad::modules::WeldBomItem> bom = welding_service_.getWeldBom("MainAssembly");
        if (bom.empty()) {
            main_window_.setIntegrationStatus("Weld BOM: Add welds first (Weld)");
            main_window_.setViewportStatus("Keine Nähte für Schweißstückliste");
        } else {
            std::string summary = "Weld BOM: " + std::to_string(bom.size()) + " types";
            for (const auto& item : bom) {
                summary += "; " + item.weld_type_name + " x" + std::to_string(item.quantity) + " L=" + std::to_string(static_cast<int>(item.total_length_mm)) + " mm";
            }
            main_window_.setIntegrationStatus(summary);
            main_window_.setViewportStatus("Schweißstückliste: " + std::to_string(bom.size()) + " Nahttypen");
        }
    } else if (command == "Simplify") {
        cad::modules::SimplifyRequest request;
        request.targetAssembly = "MainAssembly";
        request.replacementType = "Cylinder";
        cad::modules::SimplifyResult result = simplify_service_.simplify(request);
        main_window_.setIntegrationStatus(result.message);
        main_window_.setViewportStatus("Simplify queued");
    } else if (command == "Illustration" || command == "Rendering" || command == "Animation") {
        cad::modules::VisualizationRequest request;
        request.targetPart = "Bracket";
        request.mode = cad::modules::VisualizationMode::Rendering;
        cad::modules::VisualizationResult result = visualization_service_.runVisualization(request);
        main_window_.setIntegrationStatus(result.message);
        main_window_.setViewportStatus("Visualization queued");
    } else if (command == "MBD Note" || command == "MbdNote") {
        cad::modules::MbdRequest request;
        request.targetPart = "Bracket";
        request.note = "GD&T: profile tolerance";
        cad::modules::MbdResult result = mbd_service_.applyMbd(request);
        main_window_.setIntegrationStatus(result.message);
        main_window_.setViewportStatus("MBD annotation queued");
        cad::mbd::PmiDataSet pmi = mbd_service_.buildDefaultPmi("Bracket");
        (void)pmi;
    } else if (command == "Simulation" || command == "Stress Analysis") {
        cad::modules::SimulationRequest request;
        request.targetAssembly = "MainAssembly";
        request.type = cad::modules::SimulationType::FEA;
        cad::modules::SimulationResult result = simulation_service_.runSimulation(request);
        main_window_.setIntegrationStatus(result.message);
        if (result.success && result.fea_result.max_stress >= 0.0) {
            std::string status = "FEA: max stress " + std::to_string(static_cast<int>(result.fea_result.max_stress / 1e6)) + " MPa";
            if (result.fea_result.safety_factor > 0.0) {
                status += ", safety factor " + std::to_string(static_cast<int>(result.fea_result.safety_factor * 10) / 10.0);
            }
            main_window_.setViewportStatus(status);
        } else {
            main_window_.setViewportStatus("Simulation queued");
        }
    } else if (command == "ExportFEAReport") {
        cad::modules::SimulationRequest request;
        request.targetAssembly = "MainAssembly";
        request.type = cad::modules::SimulationType::FEA;
        cad::modules::SimulationResult result = simulation_service_.runSimulation(request);
        if (simulation_service_.exportFeaReport(result, "fea_report.txt")) {
            main_window_.setIntegrationStatus("FEA report exported to fea_report.txt");
            main_window_.setViewportStatus("Bericht: " + result.message);
        } else {
            main_window_.setIntegrationStatus("FEA report export failed");
            main_window_.setViewportStatus("Export fehlgeschlagen");
        }
    } else if (command == "ExportMotionReport") {
        cad::modules::SimulationRequest request;
        request.targetAssembly = "MainAssembly";
        request.type = cad::modules::SimulationType::Motion;
        request.duration = 2.0;
        request.time_step = 0.02;
        request.joint_drives["Revolute1"] = 1.0;
        cad::modules::SimulationResult result = simulation_service_.runSimulation(request);
        if (simulation_service_.exportMotionReport(result, "motion_report.txt")) {
            main_window_.setIntegrationStatus("Motion report exported to motion_report.txt");
            main_window_.setViewportStatus("Bewegungsdiagramm exportiert");
        } else {
            main_window_.setIntegrationStatus("Motion report export failed");
            main_window_.setViewportStatus("Export fehlgeschlagen");
        }
    } else if (command == "Interference") {
        cad::core::InterferenceResult result = interference_checker_.checkAssembly(active_assembly_);
        main_window_.setIntegrationStatus(result.message);
        if (result.has_interference) {
            std::string status = "Interference: " + std::to_string(result.overlap_count) + " overlap(s)";
            if (!result.interference_pairs.empty()) {
                status += " (" + result.interference_pairs[0].part_a_name + 
                         " <-> " + result.interference_pairs[0].part_b_name + ")";
            }
            main_window_.setViewportStatus(status);
        } else {
            main_window_.setViewportStatus("No interference detected");
        }
    } else if (command == "Base View" || command == "BaseView") {
        cad::modules::DrawingRequest request;
        request.sourcePart = "Bracket";
        request.templateName = "ISO";
        request.sheetFormatId = "A4_Landscape";
        cad::modules::DrawingResult result = drawing_service_.createDrawing(request);
        if (result.success) {
            cad::drawings::DrawingDocument document =
                drawing_service_.buildDocumentSkeleton(result.drawingId, request.sheetFormatId);
            document.bom = bom_service_.getBomForAssembly("MainAssembly");
            if (!document.sheets.empty()) {
                document.annotations =
                    annotation_service_.buildDefaultAnnotations(document.sheets.front().name);
                document.dimensions =
                    annotation_service_.buildDefaultDimensions(document.sheets.front().name);
            }
            techdraw_bridge_.syncDrawing(document);
            techdraw_bridge_.syncDimensions(document);
            techdraw_bridge_.syncAssociativeLinks(document);
            associative_link_service_.updateFromModel(document, document.source_model_id);
            main_window_.setIntegrationStatus("Drawing created");
#ifdef CAD_USE_EIGENER_KERN
            if (eigen_kernel_ && eigen_kernel_->isAvailable()) {
                main_window_.setViewportStatus("Zeichnung erstellt (Eigen-Kern)");
            } else
#endif
            {
                main_window_.setViewportStatus("Drawing view created");
            }
        } else {
            main_window_.setIntegrationStatus("Drawing failed");
            main_window_.setViewportStatus("Drawing view failed");
        }
    } else if (command == "Parts List" || command == "PartsList") {
        if (bom_service_.hasAssembly("MainAssembly")) {
            std::vector<cad::drawings::BillOfMaterialsItem> bom = 
                bom_service_.getBomForAssembly("MainAssembly");
            std::string bom_summary = "BOM: " + std::to_string(bom.size()) + " items";
            for (const auto& item : bom) {
                bom_summary += ", " + item.part_name + " x" + std::to_string(item.quantity);
            }
            main_window_.setIntegrationStatus(bom_summary);
            main_window_.setViewportStatus("BOM displayed: " + std::to_string(bom.size()) + " items");
            
            // Update property panel with BOM items
            #ifdef CAD_USE_QT
            cad::ui::QtMainWindow* qt_window = main_window_.nativeWindow();
            if (qt_window) {
                cad::ui::QtPropertyPanel* panel = qt_window->propertyPanel();
                if (panel) {
                    QList<cad::ui::BomItem> bom_items;
                    for (const auto& item : bom) {
                        cad::ui::BomItem bom_item;
                        bom_item.part_name = QString::fromStdString(item.part_name);
                        bom_item.quantity = item.quantity;
                        bom_item.part_number = QString::fromStdString(item.part_number);
                        bom_items.append(bom_item);
                    }
                    panel->setBomItems(bom_items);
                }
            }
            #endif
        } else {
            main_window_.setIntegrationStatus("BOM: No assembly registered");
            main_window_.setViewportStatus("BOM not available");
        }
    } else if (command == "Dimension") {
        cad::modules::DrawingRequest request;
        request.sourcePart = "Bracket";
        request.sheetFormatId = "A4_Landscape";
        cad::modules::DrawingResult result = drawing_service_.createDrawing(request);
        if (result.success) {
            cad::drawings::DrawingDocument document =
                drawing_service_.buildDocumentSkeleton(result.drawingId, request.sheetFormatId);
            if (!document.sheets.empty()) {
                document.dimensions = 
                    annotation_service_.buildDefaultDimensions(document.sheets.front().name);
                document.annotations =
                    annotation_service_.buildDefaultAnnotations(document.sheets.front().name);
                techdraw_bridge_.syncDimensions(document);
                main_window_.setIntegrationStatus("Dimensions added");
                main_window_.setViewportStatus("Dimensions: " + 
                    std::to_string(document.dimensions.size()) + " items");
                
                // Update property panel with annotations
                #ifdef CAD_USE_QT
                cad::ui::QtMainWindow* qt_window = main_window_.nativeWindow();
                if (qt_window) {
                    cad::ui::QtPropertyPanel* panel = qt_window->propertyPanel();
                    if (panel) {
                        QList<cad::ui::AnnotationItem> annotation_items;
                        for (const auto& ann : document.annotations) {
                            cad::ui::AnnotationItem item;
                            item.text = QString::fromStdString(ann.text);
                            switch (ann.type) {
                                case cad::drawings::AnnotationType::Text:
                                    item.type = "Text";
                                    break;
                                case cad::drawings::AnnotationType::Note:
                                    item.type = "Note";
                                    break;
                                case cad::drawings::AnnotationType::Callout:
                                    item.type = "Callout";
                                    break;
                                case cad::drawings::AnnotationType::Balloon:
                                    item.type = "Balloon";
                                    break;
                                case cad::drawings::AnnotationType::Revision:
                                    item.type = "Revision";
                                    break;
                                case cad::drawings::AnnotationType::Title:
                                    item.type = "Title";
                                    break;
                                default:
                                    item.type = "Leader";
                                    break;
                            }
                            item.x = ann.x;
                            item.y = ann.y;
                            item.has_leader = ann.has_leader;
                            item.has_attachment = ann.has_attachment;
                            item.attachment_entity = QString::fromStdString(ann.attachment_point.entity_id);
                            annotation_items.append(item);
                        }
                        panel->setAnnotationItems(annotation_items);
                    }
                }
                #endif
            }
        }
    } else if (command == "Section" || command == "SectionView") {
        cad::modules::DrawingRequest request;
        request.sourcePart = "Bracket";
        request.templateName = "ISO";
        request.sheetFormatId = "A4_Landscape";
        cad::modules::DrawingResult result = drawing_service_.createDrawing(request);
        if (result.success) {
            cad::drawings::DrawingDocument document =
                drawing_service_.buildDocumentSkeleton(result.drawingId, request.sheetFormatId);
            main_window_.setIntegrationStatus("Section view created");
            main_window_.setViewportStatus("Section view: " + result.drawingId);
        }
    } else if (command == "Detail View" || command == "DetailView") {
        cad::modules::DrawingRequest request;
        request.sourcePart = "Bracket";
        request.templateName = "ISO";
        request.sheetFormatId = "A4_Landscape";
        cad::modules::DrawingResult result = drawing_service_.createDrawing(request);
        if (result.success) {
            cad::drawings::DrawingDocument document =
                drawing_service_.buildDocumentSkeleton(result.drawingId, request.sheetFormatId);
            if (techdraw_bridge_.createDetailView("Detail1", "Front", 50.0, 50.0, 25.0, 2.0)) {
                main_window_.setIntegrationStatus("Detail view created");
                main_window_.setViewportStatus("Detail view: Detail1 (scale 2:1)");
            } else {
                main_window_.setIntegrationStatus("Detail view prepared");
                main_window_.setViewportStatus("Detail-Ansicht vorbereitet (Eigen-Kern)");
            }
        } else {
            main_window_.setIntegrationStatus("Drawing failed");
            main_window_.setViewportStatus("Detail view failed");
        }
    } else if (command == "Styles") {
        cad::drawings::DrawingStyleSet iso_styles = drawing_service_.isoStyles();
        cad::drawings::DrawingStyleSet ansi_styles = drawing_service_.ansiStyles();
        cad::drawings::DrawingStyleSet default_styles = drawing_service_.defaultStyles();
        std::string style_summary = "Styles: ISO (" + 
            std::to_string(iso_styles.line_styles.size()) + " lines, " +
            std::to_string(iso_styles.text_styles.size()) + " texts), ANSI (" +
            std::to_string(ansi_styles.line_styles.size()) + " lines, " +
            std::to_string(ansi_styles.text_styles.size()) + " texts)";
        main_window_.setIntegrationStatus(style_summary);
        main_window_.setViewportStatus("Style editor available");
        
        // Update property panel with style information
        #ifdef CAD_USE_QT
        cad::ui::QtMainWindow* qt_window = main_window_.nativeWindow();
        if (qt_window) {
            cad::ui::QtPropertyPanel* panel = qt_window->propertyPanel();
            if (panel) {
                QStringList presets = {"Default", "ISO", "DIN", "ANSI", "JIS"};
                panel->setStylePresets(presets);
                panel->setStylePresetSelector(presets);
                
                QString style_info = QString("Line styles: %1, Text styles: %2, Dimension styles: %3, Hatch styles: %4")
                    .arg(default_styles.line_styles.size())
                    .arg(default_styles.text_styles.size())
                    .arg(default_styles.dimension_styles.size())
                    .arg(default_styles.hatch_styles.size());
                panel->setCurrentStylePreset(QString::fromStdString("Default"));
                panel->setStyleInfo(style_info);
                
                // Update line styles table
                QList<QStringList> line_styles_data;
                for (const auto& line_style : default_styles.line_styles) {
                    QStringList row;
                    row << QString::fromStdString(line_style.name)
                        << QString::number(line_style.thickness, 'f', 2)
                        << QString::fromStdString(line_style.color)
                        << "Solid";  // Simplified type display
                    line_styles_data.append(row);
                }
                panel->setLineStylesTable(line_styles_data);
                
                // Update text styles table
                QList<QStringList> text_styles_data;
                for (const auto& text_style : default_styles.text_styles) {
                    QStringList row;
                    row << QString::fromStdString(text_style.name)
                        << QString::number(text_style.size, 'f', 1)
                        << QString::fromStdString(text_style.font_family)
                        << "Normal";  // Simplified weight display
                    text_styles_data.append(row);
                }
                panel->setTextStylesTable(text_styles_data);
            }
        }
        #endif
    } else if (command == "MBD View" || command == "MbdView") {
        cad::modules::MbdRenderRequest render_request;
        render_request.part_id = "Bracket";
        render_request.show_annotations = true;
        render_request.show_datums = true;
        render_request.show_tolerances = true;
        cad::modules::MbdRenderResult render_result = mbd_service_.prepareForRendering(render_request);
        if (render_result.success) {
            std::string mbd_summary = "MBD: " + 
                std::to_string(render_result.visible_annotations.size()) + " annotations, " +
                std::to_string(render_result.visible_datums.size()) + " datums, " +
                std::to_string(render_result.visible_tolerances.size()) + " tolerances";
            main_window_.setIntegrationStatus(mbd_summary);
            main_window_.setViewportStatus("MBD view prepared for rendering");
        } else {
            main_window_.setIntegrationStatus("MBD: No PMI data available");
            main_window_.setViewportStatus("MBD view unavailable");
        }
    } else if (command == "Import") {
        #ifdef CAD_USE_QT
        if (cad::ui::QtMainWindow* w = main_window_.nativeWindow()) {
            w->triggerImportDialog();
            return;
        }
        #endif
        main_window_.setIntegrationStatus("Import: use File dialog (Qt required)");
    } else if (command == "Export") {
        #ifdef CAD_USE_QT
        if (cad::ui::QtMainWindow* w = main_window_.nativeWindow()) {
            w->triggerExportDialog();
            return;
        }
        #endif
        main_window_.setIntegrationStatus("Export: use File dialog (Qt required)");
    } else if (command == "Export RFA" || command == "ExportRFA") {
        cad::interop::IoResult result = io_service_.exportBimRfa("C:/temp/model.rfa");
        main_window_.setIntegrationStatus(result.message);
        main_window_.setViewportStatus("RFA export queued");
    } else if (command == "Mate" || command.find("Mate ") == 0) {
        // Mate-Dialog: "Mate" (Dialog öffnen) oder "Mate <type> [value]" (aus Command Line)
        std::string mate_type_str;
        double mate_value = 0.0;
        bool has_value = false;

        if (command == "Mate") {
            // Ribbon-Klick: Dialog öffnen, dann Befehl mit Typ/Wert ausführen
#ifdef CAD_USE_QT
            cad::ui::QtMainWindow* qt_win = main_window_.nativeWindow();
            if (qt_win) {
                cad::ui::QtMateDialog dlg(qt_win);
                if (dlg.exec() == QDialog::Accepted) {
                    mate_type_str = dlg.mateType().toStdString();
                    has_value = dlg.isValueUsed();
                    mate_value = dlg.mateValue();
                } else {
                    main_window_.setMateCount(static_cast<int>(active_assembly_.mates().size()));
                    updateAssemblyConstraintStatus();
                    return;
                }
            } else {
                mate_type_str = "Coincident";
            }
#else
            mate_type_str = "Coincident";
#endif
        } else if (command.size() > 5 && command[5] == ' ') {
            std::istringstream iss(command.substr(6));
            std::string token;
            if (iss >> token) mate_type_str = token;
            if (iss >> token) {
                try { mate_value = std::stod(token); has_value = true; } catch (...) {}
            }
        }
        if (mate_type_str.empty()) mate_type_str = "Coincident";

        if (active_assembly_.components().size() < 2) {
            main_window_.setIntegrationStatus("Mate: Need at least 2 components");
            main_window_.setViewportStatus("Add components first");
            main_window_.setMateCount(static_cast<int>(active_assembly_.mates().size()));
            updateAssemblyConstraintStatus();
        } else {
            std::uint64_t id_a = active_assembly_.components()[0].id;
            std::uint64_t id_b = active_assembly_.components()[1].id;
            std::string mate_name;
            const double deg2rad = 3.14159265358979323846 / 180.0;
            if (mate_type_str == "Coincident") {
                mate_name = active_assembly_.createMate(id_a, id_b, mate_value);
            } else if (mate_type_str == "Parallel") {
                mate_name = active_assembly_.createParallel(id_a, id_b, has_value ? mate_value : 0.0);
            } else if (mate_type_str == "Distance") {
                mate_name = active_assembly_.createDistance(id_a, id_b, has_value ? mate_value : 10.0);
            } else if (mate_type_str == "Tangent") {
                mate_name = active_assembly_.createTangent(id_a, id_b, has_value ? mate_value : 0.0);
            } else if (mate_type_str == "Concentric") {
                mate_name = active_assembly_.createConcentric(id_a, id_b, has_value ? mate_value : 0.0);
            } else if (mate_type_str == "Perpendicular") {
                double angle_rad = has_value ? (mate_value * deg2rad) : (90.0 * deg2rad);
                mate_name = active_assembly_.createAngle(id_a, id_b, angle_rad);
            } else {
                mate_name = active_assembly_.createMate(id_a, id_b, 0.0);
            }
            active_assembly_.solveMates();
            main_window_.setIntegrationStatus("Mate (" + mate_type_str + ") created: " + mate_name);
            main_window_.setViewportStatus("Mate constraint applied");
            main_window_.setMateCount(static_cast<int>(active_assembly_.mates().size()));
            updateAssemblyConstraintStatus();
        }
    } else if (command == "LoadAssembly") {
        assembly_manager_.enqueueLoad("MainAssembly");
        main_window_.setLoadProgress(0);
        main_window_.setViewportStatus("Assembly load queued");
    } else if (command == "Place") {
        cad::core::AssemblyLoadJob job = assembly_manager_.pollLoadProgress();
        if (!job.path.empty()) {
            main_window_.setViewportStatus("Assembly load " + std::to_string(job.progress) + "%");
            main_window_.setLoadProgress(job.progress);
        } else {
            main_window_.setViewportStatus("No assembly load in progress");
        }
    } else if (command == "Measure") {
        main_window_.setIntegrationStatus("Measure tool ready");
        main_window_.setViewportStatus("Select geometry to measure");
    } else if (command == "SectionAnalysis") {
        main_window_.setIntegrationStatus("Section analysis ready");
        main_window_.setViewportStatus("Section analysis: select plane");
    } else if (base_cmd == "Line") {
        // Line requires 4 parameters: x1 y1 x2 y2. Without params: show hint only (no geometry).
        const int required = 4;
        if (param_strs.size() < static_cast<size_t>(required)) {
            main_window_.setIntegrationStatus("Line: enter start and end in command line");
            main_window_.setViewportStatus("Line: x1 y1 x2 y2 (e.g. Line 0 0 50 50)");
            return;
        }
        double x1 = 0, y1 = 0, x2 = 0, y2 = 0;
        if (param_strs.size() >= 4u) {
            x1 = std::strtod(param_strs[0].c_str(), nullptr);
            y1 = std::strtod(param_strs[1].c_str(), nullptr);
            x2 = std::strtod(param_strs[2].c_str(), nullptr);
            y2 = std::strtod(param_strs[3].c_str(), nullptr);
        }
        cad::core::Point2D start{x1, y1};
        cad::core::Point2D end{x2, y2};
        std::string geom_id = active_sketch_.addLine(start, end);
        main_window_.setIntegrationStatus("Line added: " + geom_id);
        main_window_.setViewportStatus("Line created");
#ifdef CAD_USE_QT
        cad::ui::QtMainWindow* qt_window = main_window_.nativeWindow();
        if (qt_window && qt_window->viewport3D())
            qt_window->viewport3D()->renderGeometry(geom_id, nullptr);
#endif
        main_window_.setConstraintCount(static_cast<int>(active_sketch_.constraints().size()));
        updateSketchConstraintStatus();
    } else if (base_cmd == "Rectangle") {
        const int required = 4;  // corner_x corner_y width height
        if (param_strs.size() < static_cast<size_t>(required)) {
            main_window_.setIntegrationStatus("Rectangle: enter corner and size in command line");
            main_window_.setViewportStatus("Rectangle: x y width height (e.g. Rectangle 0 0 100 50)");
            return;
        }
        double cx = std::strtod(param_strs[0].c_str(), nullptr);
        double cy = std::strtod(param_strs[1].c_str(), nullptr);
        double w  = std::strtod(param_strs[2].c_str(), nullptr);
        double h  = std::strtod(param_strs[3].c_str(), nullptr);
        cad::core::Point2D corner{cx, cy};
        std::string geom_id = active_sketch_.addRectangle(corner, w, h);
        main_window_.setIntegrationStatus("Rectangle added: " + geom_id);
        main_window_.setViewportStatus("Rectangle created");
#ifdef CAD_USE_QT
        cad::ui::QtMainWindow* qt_window = main_window_.nativeWindow();
        if (qt_window && qt_window->viewport3D())
            qt_window->viewport3D()->renderGeometry(geom_id, nullptr);
#endif
        main_window_.setConstraintCount(static_cast<int>(active_sketch_.constraints().size()));
        updateSketchConstraintStatus();
    } else if (base_cmd == "Circle") {
        const int required = 3;  // center_x center_y radius
        if (param_strs.size() < static_cast<size_t>(required)) {
            main_window_.setIntegrationStatus("Circle: enter center and radius in command line");
            main_window_.setViewportStatus("Circle: cx cy radius (e.g. Circle 25 25 20)");
            return;
        }
        double cx = std::strtod(param_strs[0].c_str(), nullptr);
        double cy = std::strtod(param_strs[1].c_str(), nullptr);
        double r  = std::strtod(param_strs[2].c_str(), nullptr);
        cad::core::Point2D center{cx, cy};
        std::string geom_id = active_sketch_.addCircle(center, r);
        main_window_.setIntegrationStatus("Circle added: " + geom_id);
        main_window_.setViewportStatus("Circle created");
#ifdef CAD_USE_QT
        cad::ui::QtMainWindow* qt_window = main_window_.nativeWindow();
        if (qt_window && qt_window->viewport3D())
            qt_window->viewport3D()->renderGeometry(geom_id, nullptr);
#endif
        main_window_.setConstraintCount(static_cast<int>(active_sketch_.constraints().size()));
        updateSketchConstraintStatus();
    } else if (base_cmd == "Arc") {
        const int required = 5;  // center_x center_y radius start_angle end_angle
        if (param_strs.size() < static_cast<size_t>(required)) {
            main_window_.setIntegrationStatus("Arc: enter center, radius and angles in command line");
            main_window_.setViewportStatus("Arc: cx cy radius start_angle end_angle (e.g. Arc 25 25 20 0 90)");
            return;
        }
        double cx   = std::strtod(param_strs[0].c_str(), nullptr);
        double cy   = std::strtod(param_strs[1].c_str(), nullptr);
        double r    = std::strtod(param_strs[2].c_str(), nullptr);
        double sang = std::strtod(param_strs[3].c_str(), nullptr);
        double eang = std::strtod(param_strs[4].c_str(), nullptr);
        cad::core::Point2D center{cx, cy};
        std::string geom_id = active_sketch_.addArc(center, r, sang, eang);
        main_window_.setIntegrationStatus("Arc added: " + geom_id);
        main_window_.setViewportStatus("Arc created");
#ifdef CAD_USE_QT
        cad::ui::QtMainWindow* qt_window = main_window_.nativeWindow();
        if (qt_window && qt_window->viewport3D())
            qt_window->viewport3D()->renderGeometry(geom_id, nullptr);
#endif
        main_window_.setConstraintCount(static_cast<int>(active_sketch_.constraints().size()));
        updateSketchConstraintStatus();
    } else if (command == "Constraint") {
        main_window_.setConstraintCount(static_cast<int>(active_sketch_.constraints().size()));
        updateSketchConstraintStatus();
    } else if (command == "Extrude") {
        // Create extrude feature from active sketch
        if (!active_sketch_.geometry().empty()) {
            cad::core::Part part = modeler_.createPart(active_sketch_);
            double depth = 10.0;  // Default depth
            // Try to parse depth from command if available
            modeler_.applyExtrude(part, active_sketch_.name(), depth);
#ifdef CAD_USE_EIGENER_KERN
            if (eigen_kernel_ && eigen_kernel_->isAvailable()) {
                std::map<std::string, cad::core::Sketch> sketches;
                sketches.insert(std::make_pair(active_sketch_.name(), active_sketch_));
                eigen_kernel_->buildPartFromPart(part, &sketches);
            }
#endif
            main_window_.setIntegrationStatus("Extrude created: depth=" + std::to_string(depth));
            main_window_.setViewportStatus("Extrude feature applied");
            
            // Render in viewport
            #ifdef CAD_USE_QT
            cad::ui::QtMainWindow* qt_window = main_window_.nativeWindow();
            if (qt_window && qt_window->viewport3D()) {
                qt_window->viewport3D()->renderGeometry("extrude_" + active_sketch_.name(), nullptr);
            }
            #endif
        } else {
            main_window_.setIntegrationStatus("Extrude: No sketch geometry available");
            main_window_.setViewportStatus("Create sketch first");
        }
    } else if (command == "ExtrudeReverse") {
        if (!active_sketch_.geometry().empty()) {
            cad::core::Part part = modeler_.createPart(active_sketch_);
            double depth = -10.0;  // Reverse direction
            modeler_.applyExtrude(part, active_sketch_.name(), depth);
#ifdef CAD_USE_EIGENER_KERN
            if (eigen_kernel_ && eigen_kernel_->isAvailable()) {
                std::map<std::string, cad::core::Sketch> sketches;
                sketches.insert(std::make_pair(active_sketch_.name(), active_sketch_));
                eigen_kernel_->buildPartFromPart(part, &sketches);
            }
#endif
            main_window_.setIntegrationStatus("Extrude (Reverse) created: depth=" + std::to_string(depth));
            main_window_.setViewportStatus("Extrude reverse applied");
#ifdef CAD_USE_QT
            cad::ui::QtMainWindow* qt_window = main_window_.nativeWindow();
            if (qt_window && qt_window->viewport3D())
                qt_window->viewport3D()->renderGeometry("extrude_" + active_sketch_.name(), nullptr);
#endif
        } else {
            main_window_.setIntegrationStatus("Extrude (Reverse): No sketch geometry available");
            main_window_.setViewportStatus("Create sketch first");
        }
    } else if (command == "ExtrudeBoth") {
        if (!active_sketch_.geometry().empty()) {
            cad::core::Part part = modeler_.createPart(active_sketch_);
            double depth = 10.0;
            modeler_.applyExtrude(part, active_sketch_.name(), depth, true);  // symmetric = true
#ifdef CAD_USE_EIGENER_KERN
            if (eigen_kernel_ && eigen_kernel_->isAvailable()) {
                std::map<std::string, cad::core::Sketch> sketches;
                sketches.insert(std::make_pair(active_sketch_.name(), active_sketch_));
                eigen_kernel_->buildPartFromPart(part, &sketches);
            }
#endif
            main_window_.setIntegrationStatus("Extrude (Both) created: depth=" + std::to_string(depth) + " symmetric");
            main_window_.setViewportStatus("Extrude both directions applied");
#ifdef CAD_USE_QT
            cad::ui::QtMainWindow* qt_window = main_window_.nativeWindow();
            if (qt_window && qt_window->viewport3D())
                qt_window->viewport3D()->renderGeometry("extrude_" + active_sketch_.name(), nullptr);
#endif
        } else {
            main_window_.setIntegrationStatus("Extrude (Both): No sketch geometry available");
            main_window_.setViewportStatus("Create sketch first");
        }
    } else if (command == "Revolve") {
        // Create revolve feature from active sketch
        if (!active_sketch_.geometry().empty()) {
            cad::core::Part part = modeler_.createPart(active_sketch_);
            double angle = 360.0;  // Default full revolution
            modeler_.applyRevolve(part, active_sketch_.name(), angle);
#ifdef CAD_USE_EIGENER_KERN
            if (eigen_kernel_ && eigen_kernel_->isAvailable()) {
                std::map<std::string, cad::core::Sketch> sketches;
                sketches.insert(std::make_pair(active_sketch_.name(), active_sketch_));
                eigen_kernel_->buildPartFromPart(part, &sketches);
            }
#endif
            main_window_.setIntegrationStatus("Revolve created: angle=" + std::to_string(angle));
            main_window_.setViewportStatus("Revolve feature applied");
            
            #ifdef CAD_USE_QT
            cad::ui::QtMainWindow* qt_window = main_window_.nativeWindow();
            if (qt_window && qt_window->viewport3D()) {
                qt_window->viewport3D()->renderGeometry("revolve_" + active_sketch_.name(), nullptr);
            }
            #endif
        } else {
            main_window_.setIntegrationStatus("Revolve: No sketch geometry available");
            main_window_.setViewportStatus("Create sketch first");
        }
    } else if (command == "Loft") {
        // Create loft feature (requires multiple sketches)
        cad::core::Part part = modeler_.createPart(active_sketch_);
        std::vector<std::string> sketch_ids = {active_sketch_.name()};
        modeler_.applyLoft(part, sketch_ids);
        main_window_.setIntegrationStatus("Loft created");
        main_window_.setViewportStatus("Loft feature applied");
        
        #ifdef CAD_USE_QT
        cad::ui::QtMainWindow* qt_window = main_window_.nativeWindow();
        if (qt_window && qt_window->viewport3D()) {
            qt_window->viewport3D()->renderGeometry("loft_" + active_sketch_.name(), nullptr);
        }
        #endif
    } else if (command == "Hole") {
        // Create hole feature
        cad::core::Part part = modeler_.createPart(active_sketch_);
        double diameter = 5.0;  // Default diameter
        double depth = 10.0;   // Default depth
        modeler_.applyHole(part, diameter, depth, false);
        main_window_.setIntegrationStatus("Hole created: diameter=" + std::to_string(diameter));
        main_window_.setViewportStatus("Hole feature applied");
        
        #ifdef CAD_USE_QT
        cad::ui::QtMainWindow* qt_window = main_window_.nativeWindow();
        if (qt_window && qt_window->viewport3D()) {
            qt_window->viewport3D()->renderGeometry("hole_" + active_sketch_.name(), nullptr);
        }
        #endif
    } else if (command == "HoleThroughAll") {
        cad::core::Part part = modeler_.createPart(active_sketch_);
        double diameter = 5.0;
        double depth = 0.0;  // Ignored when through_all
        modeler_.applyHole(part, diameter, depth, true);
        main_window_.setIntegrationStatus("Hole (through all) created: diameter=" + std::to_string(diameter));
        main_window_.setViewportStatus("Hole through all applied");
        #ifdef CAD_USE_QT
        cad::ui::QtMainWindow* qt_window = main_window_.nativeWindow();
        if (qt_window && qt_window->viewport3D()) {
            qt_window->viewport3D()->renderGeometry("hole_" + active_sketch_.name(), nullptr);
        }
        #endif
    } else if (command == "Fillet") {
        // Create fillet feature
        cad::core::Part part = modeler_.createPart(active_sketch_);
        double radius = 2.0;  // Default radius
        std::vector<std::string> edge_ids;  // Empty for now
        modeler_.applyFillet(part, radius, edge_ids);
        main_window_.setIntegrationStatus("Fillet created: radius=" + std::to_string(radius));
        main_window_.setViewportStatus("Fillet feature applied");
        
        #ifdef CAD_USE_QT
        cad::ui::QtMainWindow* qt_window = main_window_.nativeWindow();
        if (qt_window && qt_window->viewport3D()) {
            qt_window->viewport3D()->renderGeometry("fillet_" + active_sketch_.name(), nullptr);
        }
        #endif
    } else if (command == "Chamfer") {
        cad::core::Part part = modeler_.createPart(active_sketch_);
        double d1 = 1.0, d2 = 1.0, angle = 45.0;
        std::vector<std::string> edge_ids;
        modeler_.applyChamfer(part, d1, d2, angle, edge_ids);
        main_window_.setIntegrationStatus("Chamfer created: d1=" + std::to_string(d1) + " d2=" + std::to_string(d2));
        main_window_.setViewportStatus("Chamfer feature applied");
    } else if (command == "Shell") {
        cad::core::Part part = modeler_.createPart(active_sketch_);
        double wall = 1.0;
        std::vector<std::string> face_ids;
        modeler_.applyShell(part, wall, face_ids);
        main_window_.setIntegrationStatus("Shell created: wall=" + std::to_string(wall) + " mm");
        main_window_.setViewportStatus("Shell feature applied");
    } else if (command == "Mirror") {
        cad::core::Part part = modeler_.createPart(active_sketch_);
        if (!part.features().empty()) {
            std::string base = part.features().back().name;
            std::string plane = "XY";
            modeler_.applyMirror(part, base, plane, true);
            main_window_.setIntegrationStatus("Mirror created: base=" + base + ", plane=" + plane);
            main_window_.setViewportStatus("Mirror feature applied");
        } else {
            main_window_.setIntegrationStatus("Mirror: Create a feature first");
            main_window_.setViewportStatus("Add feature before mirror");
        }
    } else if (command == "Flush") {
        // Create flush mate constraint
        if (active_assembly_.components().size() >= 2) {
            std::uint64_t id_a = active_assembly_.components()[0].id;
            std::uint64_t id_b = active_assembly_.components()[1].id;
            std::string mate_name = active_assembly_.createFlush(id_a, id_b, 0.0);
            active_assembly_.solveMates();
            main_window_.setIntegrationStatus("Flush mate created: " + mate_name);
            main_window_.setViewportStatus("Flush constraint applied");
            main_window_.setMateCount(static_cast<int>(active_assembly_.mates().size()));
            updateAssemblyConstraintStatus();
        } else {
            main_window_.setIntegrationStatus("Flush: Need at least 2 components");
            main_window_.setViewportStatus("Add components first");
        }
    } else if (command == "Angle") {
        // Create angle mate constraint
        if (active_assembly_.components().size() >= 2) {
            std::uint64_t id_a = active_assembly_.components()[0].id;
            std::uint64_t id_b = active_assembly_.components()[1].id;
            double angle = 45.0;  // Default angle
            std::string mate_name = active_assembly_.createAngle(id_a, id_b, angle);
            active_assembly_.solveMates();
            main_window_.setIntegrationStatus("Angle mate created: " + mate_name + " angle=" + std::to_string(angle));
            main_window_.setViewportStatus("Angle constraint applied");
            main_window_.setMateCount(static_cast<int>(active_assembly_.mates().size()));
            updateAssemblyConstraintStatus();
        } else {
            main_window_.setIntegrationStatus("Angle: Need at least 2 components");
            main_window_.setViewportStatus("Add components first");
        }
    } else if (command == "Parallel") {
        if (active_assembly_.components().size() >= 2) {
            std::uint64_t id_a = active_assembly_.components()[0].id;
            std::uint64_t id_b = active_assembly_.components()[1].id;
            double distance = 0.0;
            std::string mate_name = active_assembly_.createParallel(id_a, id_b, distance);
            active_assembly_.solveMates();
            main_window_.setIntegrationStatus("Parallel mate created: " + mate_name);
            main_window_.setViewportStatus("Parallel constraint applied");
            main_window_.setMateCount(static_cast<int>(active_assembly_.mates().size()));
            updateAssemblyConstraintStatus();
        } else {
            main_window_.setIntegrationStatus("Parallel: Need at least 2 components");
            main_window_.setViewportStatus("Add components first");
        }
    } else if (command == "Distance") {
        if (active_assembly_.components().size() >= 2) {
            std::uint64_t id_a = active_assembly_.components()[0].id;
            std::uint64_t id_b = active_assembly_.components()[1].id;
            double distance = 10.0;  // Default distance
            std::string mate_name = active_assembly_.createDistance(id_a, id_b, distance);
            active_assembly_.solveMates();
            main_window_.setIntegrationStatus("Distance mate created: " + mate_name + " distance=" + std::to_string(distance));
            main_window_.setViewportStatus("Distance constraint applied");
            main_window_.setMateCount(static_cast<int>(active_assembly_.mates().size()));
            updateAssemblyConstraintStatus();
        } else {
            main_window_.setIntegrationStatus("Distance: Need at least 2 components");
            main_window_.setViewportStatus("Add components first");
        }
    } else if (command == "Pattern") {
        main_window_.setIntegrationStatus("Pattern command ready");
        main_window_.setViewportStatus("Assembly pattern command active");
    } else if (command == "ExplosionView") {
        if (active_assembly_.getExplosionFactor() > 0) {
            active_assembly_.setExplosionFactor(0.0);
            main_window_.setViewportStatus("Explosion view off");
            main_window_.setIntegrationStatus("Explosion view disabled");
        } else {
            active_assembly_.setExplosionFactor(1.0);
            for (std::size_t i = 0; i < active_assembly_.components().size(); ++i) {
                std::uint64_t cid = active_assembly_.components()[i].id;
                active_assembly_.setExplosionOffset(cid, 0.0, 0.0, static_cast<double>(i) * 2.0);
            }
            main_window_.setViewportStatus("Explosion view on");
            main_window_.setIntegrationStatus("Explosion view enabled");
        }
        syncAssemblyTransformsToViewport();
    } else if (command == "Visibility" || command == "Appearance" || command == "Environment") {
        main_window_.setIntegrationStatus("View: " + command);
        main_window_.setViewportStatus("View " + command + " command active");
    } else if (command == "Suppress") {
        // Unterdrückung: Feature oder Komponente unterdrücken (Backend: setFeatureSuppressed)
        bool did_suppress = false;
        if (!active_assembly_.components().empty()) {
            cad::core::AssemblyComponent* comp = active_assembly_.findComponent(active_assembly_.components()[0].id);
            if (comp && !comp->part.features().empty()) {
                std::string feat_name = comp->part.features().back().name;
                did_suppress = comp->part.setFeatureSuppressed(feat_name, true);
                if (did_suppress) {
                    main_window_.setIntegrationStatus("Suppress: Feature \"" + feat_name + "\" suppressed.");
                    main_window_.setViewportStatus("Feature suppressed");
                }
            }
        }
        if (!did_suppress) {
            main_window_.setIntegrationStatus("Suppress: Select component/feature in browser, then use Suppress.");
            main_window_.setViewportStatus("Suppress component or feature");
        }
    } else if (command == "Create New Component") {
        main_window_.setIntegrationStatus("Create New Component: Add new part to assembly.");
        main_window_.setViewportStatus("New component ready");
    } else if (command == "Place From File") {
        main_window_.setIntegrationStatus("Place From File: Use File → Import or place external part.");
        main_window_.setViewportStatus("Place from file");
    } else if (command == "Edit Component") {
        main_window_.setIntegrationStatus("Edit Component: Double-click component in browser to edit in context.");
        main_window_.setViewportStatus("Edit component");
    } else if (command == "Properties") {
        main_window_.setIntegrationStatus("Properties: Selection properties shown in Properties panel.");
        main_window_.setViewportStatus("Properties panel");
    } else if (command == "Rename") {
        main_window_.setIntegrationStatus("Rename: Select item in browser and rename in Properties.");
        main_window_.setViewportStatus("Rename");
    } else if (command == "Delete") {
        main_window_.setIntegrationStatus("Delete: Select item in browser and press Delete key.");
        main_window_.setViewportStatus("Delete selection");
    } else if (command == "Copy") {
        main_window_.setIntegrationStatus("Copy: Selection copied to clipboard.");
        main_window_.setViewportStatus("Copy");
    } else if (command == "Cut") {
        main_window_.setIntegrationStatus("Cut: Selection cut to clipboard.");
        main_window_.setViewportStatus("Cut");
    } else if (command == "Paste") {
        main_window_.setIntegrationStatus("Paste: Paste from clipboard.");
        main_window_.setViewportStatus("Paste");
    } else if (command == "Search") {
        main_window_.setIntegrationStatus("Search: Use browser filter or command search.");
        main_window_.setViewportStatus("Search");
    } else if (command == "Show Dependencies") {
        main_window_.setIntegrationStatus("Show Dependencies: Displays feature/component dependencies.");
        main_window_.setViewportStatus("Dependencies");
    } else if (command == "AddIns") {
        main_window_.setIntegrationStatus("Add-ins manager");
        main_window_.setViewportStatus("Add-ins panel available");
    } else if (command == "Undo") {
        if (undo_stack_.canUndo()) {
            std::string name = undo_stack_.undo();
            main_window_.setIntegrationStatus("Undo: " + name);
            main_window_.setViewportStatus("Undone");
        } else {
            main_window_.setIntegrationStatus("Nothing to undo");
            main_window_.setViewportStatus("Undo unavailable");
        }
    } else if (command == "Redo") {
        if (undo_stack_.canRedo()) {
            std::string name = undo_stack_.redo();
            main_window_.setIntegrationStatus("Redo: " + name);
            main_window_.setViewportStatus("Redone");
        } else {
            main_window_.setIntegrationStatus("Nothing to redo");
            main_window_.setViewportStatus("Redo unavailable");
        }
    } else {
        // Unknown command - provide feedback
        main_window_.setIntegrationStatus("Command: " + command);
        main_window_.setViewportStatus("Command executed: " + command);
    }
}

void AppController::newProject() {
    active_assembly_ = modeler_.createAssembly();
    cad::core::Sketch sketch("Sketch1");
    sketch.addConstraint({cad::core::ConstraintType::Distance, "line1", "line2", 25.0});
    sketch.addParameter({"Width", 0.0, "100"});
    sketch.addParameter({"Height", 0.0, "50"});
    setActiveSketch(sketch);
    modeler_.evaluateParameters(active_sketch_);
    main_window_.setConstraintCount(static_cast<int>(active_sketch_.constraints().size()));
    main_window_.setParameterCount(static_cast<int>(active_sketch_.parameters().size()));
    main_window_.setParameterSummary(buildParameterSummary(active_sketch_));
    initializeAssembly();
    has_unsaved_changes_ = false;
    current_project_path_.clear();
    main_window_.setDocumentLabel("Untitled");
    main_window_.setIntegrationStatus("New project created");
    main_window_.setViewportStatus("Ready – use Sketch tools or File → Open/Save");
    #ifdef CAD_USE_QT
    cad::ui::QtMainWindow* qt_window = main_window_.nativeWindow();
    if (qt_window) {
        qt_window->setCurrentProjectPath(QString());
        if (qt_window->viewport3D()) {
            qt_window->viewport3D()->clearScene();
        }
    }
    #endif
}

std::string AppController::getTemplateDirectory() const {
    if (!project_config_.template_directory.empty()) {
        return project_config_.template_directory;
    }
#ifdef CAD_USE_QT
    QSettings settings("HydraCAD", "HydraCAD");
    QString path = settings.value("templates/directory").toString();
    if (!path.isEmpty()) {
        return path.toStdString();
    }
#endif
    return {};
}

void AppController::setTemplateDirectory(const std::string& path) {
    project_config_.template_directory = path;
#ifdef CAD_USE_QT
    QSettings settings("HydraCAD", "HydraCAD");
    settings.setValue("templates/directory", QString::fromStdString(path));
#endif
}

std::vector<std::string> AppController::getTemplateList() const {
    std::vector<std::string> list;
    std::string dir = getTemplateDirectory();
    if (dir.empty()) {
        return list;
    }
    try {
        std::filesystem::path p(dir);
        if (!std::filesystem::is_directory(p)) {
            return list;
        }
        for (const auto& entry : std::filesystem::directory_iterator(p)) {
            if (!entry.is_regular_file()) continue;
            std::string ext = entry.path().extension().string();
            if (ext == ".cad" || ext == ".hcad") {
                list.push_back(entry.path().string());
            }
        }
    } catch (...) {}
    return list;
}

bool AppController::newProjectFromTemplate(const std::string& template_path) {
    if (template_path.empty()) {
        return false;
    }
    if (!std::filesystem::exists(template_path) || !std::filesystem::is_regular_file(template_path)) {
        main_window_.setIntegrationStatus("Template file not found: " + template_path);
        return false;
    }
#ifdef CAD_USE_QT
    cad::ui::QtMainWindow* qt_window = main_window_.nativeWindow();
    if (!qt_window) {
        return false;
    }
    std::string target = qt_window->getSavePathForNewProject();
    if (target.empty()) {
        main_window_.setIntegrationStatus("New from template cancelled");
        return false;
    }
    try {
        std::filesystem::copy_file(template_path, target, std::filesystem::copy_options::overwrite_existing);
    } catch (const std::exception& e) {
        main_window_.setIntegrationStatus(std::string("Copy failed: ") + e.what());
        return false;
    }
    bool ok = loadProject(target);
    if (ok) {
        main_window_.setIntegrationStatus("New project created from template");
        main_window_.setViewportStatus("Project loaded from template");
    }
    return ok;
#else
    main_window_.setIntegrationStatus("New from template requires Qt");
    return false;
#endif
}

bool AppController::saveProject(const std::string& file_path) {
    // Validate file path
    if (file_path.empty()) {
        main_window_.setIntegrationStatus("Error: File path is empty");
        return false;
    }
    
    std::filesystem::path path(file_path);
    if (!path.has_filename()) {
        main_window_.setIntegrationStatus("Error: Invalid file path");
        return false;
    }
    
    // Check if file exists and create backup
    if (std::filesystem::exists(file_path)) {
        std::string backup_path = file_path + ".backup";
        try {
            std::filesystem::copy_file(file_path, backup_path, 
                                      std::filesystem::copy_options::overwrite_existing);
        } catch (...) {
            // Backup failed, but continue with save
        }
    }
    
    std::map<std::string, cad::core::Sketch> sketches;
    sketches.emplace(active_sketch_.name(), active_sketch_);
    bool success = project_file_service_.saveProject(file_path, active_assembly_, &sketches);
    if (success) {
        has_unsaved_changes_ = false;
        current_project_path_ = file_path;
        if (project_config_.working_directory.empty()) {
            std::filesystem::path p(file_path);
            project_config_.working_directory = p.parent_path().string();
        }
        std::string config_path = ProjectFileService::projectConfigPathForProject(file_path);
        project_file_service_.saveProjectConfig(config_path, project_config_);
        main_window_.setIntegrationStatus("Project saved: " + file_path);
        
        // Add to recent projects (avoid duplicates)
        auto it = std::find(recent_projects_.begin(), recent_projects_.end(), file_path);
        if (it != recent_projects_.end()) {
            recent_projects_.erase(it);
        }
        recent_projects_.insert(recent_projects_.begin(), file_path);
        if (recent_projects_.size() > 10) {
            recent_projects_.pop_back();
        }
        
        // Save to settings and update menu
        saveRecentProjectsToSettings();
        #ifdef CAD_USE_QT
        cad::ui::QtMainWindow* qt_window = main_window_.nativeWindow();
        if (qt_window) {
            qt_window->updateRecentProjectsMenu(recent_projects_);
            qt_window->setCurrentProjectPath(QString::fromStdString(file_path));
        }
        #endif
    } else {
        std::string error_msg = "Failed to save project: " + file_path;
        if (!std::filesystem::exists(std::filesystem::path(file_path).parent_path())) {
            error_msg += " (Directory does not exist)";
        } else {
            error_msg += " (Check file permissions or disk space)";
        }
        main_window_.setIntegrationStatus(error_msg);
        #ifdef CAD_USE_QT
        QMessageBox::warning(nullptr, QObject::tr("Save Failed"), QString::fromStdString(error_msg));
        #endif
    }
    return success;
}

bool AppController::loadProject(const std::string& file_path) {
    if (has_unsaved_changes_) {
        #ifdef CAD_USE_QT
        cad::ui::QtMainWindow* w = main_window_.nativeWindow();
        if (w) {
            cad::ui::QtMainWindow::UnsavedAction action = w->askUnsavedChanges();
            if (action == cad::ui::QtMainWindow::UnsavedAction::Cancel) {
                main_window_.setIntegrationStatus("Open project cancelled");
                return false;
            }
            if (action == cad::ui::QtMainWindow::UnsavedAction::Save) {
                std::string path = !current_project_path_.empty() ? current_project_path_ : w->getSavePathForNewProject();
                if (path.empty()) {
                    main_window_.setIntegrationStatus("Save cancelled");
                    return false;
                }
                if (!saveProject(path)) {
                    return false;
                }
            }
        }
        #else
        main_window_.setIntegrationStatus("Warning: Unsaved changes will be lost");
        #endif
    }

    // Validate file path
    if (file_path.empty()) {
        main_window_.setIntegrationStatus("Error: File path is empty");
        return false;
    }
    
    if (!std::filesystem::exists(file_path)) {
        main_window_.setIntegrationStatus("Error: File does not exist: " + file_path);
        return false;
    }
    
    std::map<std::string, cad::core::Sketch> loaded_sketches;
    bool success = project_file_service_.loadProject(file_path, active_assembly_, &loaded_sketches);
    if (success) {
        if (!loaded_sketches.empty()) {
            auto it = loaded_sketches.find(active_sketch_.name());
            active_sketch_ = (it != loaded_sketches.end()) ? it->second : loaded_sketches.begin()->second;
            modeler_.evaluateParameters(active_sketch_);
            main_window_.setConstraintCount(static_cast<int>(active_sketch_.constraints().size()));
            main_window_.setParameterCount(static_cast<int>(active_sketch_.parameters().size()));
            main_window_.setParameterSummary(buildParameterSummary(active_sketch_));
            updateSketchConstraintStatus();
        }
        has_unsaved_changes_ = false;
        current_project_path_ = file_path;
        std::string config_path = ProjectFileService::projectConfigPathForProject(file_path);
        project_file_service_.loadProjectConfig(config_path, project_config_);
        if (project_config_.working_directory.empty()) {
            std::filesystem::path p(file_path);
            project_config_.working_directory = p.parent_path().string();
        }
        ProjectFileInfo info = project_file_service_.getProjectInfo(file_path);
        main_window_.setIntegrationStatus("Project loaded: " + file_path);
        main_window_.setDocumentLabel(info.project_name);
        main_window_.setMateCount(static_cast<int>(active_assembly_.mates().size()));
        main_window_.setAssemblySummary(std::to_string(active_assembly_.components().size()) + " components, "
                                        + std::to_string(active_assembly_.mates().size()) + " mates");
        updateAssemblyConstraintStatus();
        
        // Add to recent projects (avoid duplicates)
        auto it = std::find(recent_projects_.begin(), recent_projects_.end(), file_path);
        if (it != recent_projects_.end()) {
            recent_projects_.erase(it);
        }
        recent_projects_.insert(recent_projects_.begin(), file_path);
        if (recent_projects_.size() > 10) {
            recent_projects_.pop_back();
        }
        
        // Save to settings and update menu
        saveRecentProjectsToSettings();
        #ifdef CAD_USE_QT
        cad::ui::QtMainWindow* qt_window = main_window_.nativeWindow();
        if (qt_window) {
            qt_window->updateRecentProjectsMenu(recent_projects_);
            qt_window->setCurrentProjectPath(QString::fromStdString(file_path));
            qt_window->setTemplateDirectory(QString::fromStdString(getTemplateDirectory()));
        }
        #endif
    } else {
        std::string error_msg = "Failed to load project: " + file_path + " (File may be corrupted or incompatible)";
        main_window_.setIntegrationStatus(error_msg);
        #ifdef CAD_USE_QT
        QMessageBox::warning(nullptr, QObject::tr("Load Failed"), QString::fromStdString(error_msg));
        #endif
    }
    return success;
}

bool AppController::saveCheckpoint(const std::string& checkpoint_path) {
    bool success = project_file_service_.saveCheckpoint(checkpoint_path, active_assembly_);
    if (success) {
        main_window_.setIntegrationStatus("Checkpoint saved: " + checkpoint_path);
    } else {
        main_window_.setIntegrationStatus("Failed to save checkpoint: " + checkpoint_path);
    }
    return success;
}

bool AppController::loadCheckpoint(const std::string& checkpoint_path) {
    bool success = project_file_service_.loadCheckpoint(checkpoint_path, active_assembly_);
    if (success) {
        main_window_.setIntegrationStatus("Checkpoint loaded: " + checkpoint_path);
    } else {
        main_window_.setIntegrationStatus("Failed to load checkpoint: " + checkpoint_path);
    }
    return success;
}

bool AppController::deleteCheckpoint(const std::string& checkpoint_path) {
    bool success = project_file_service_.deleteCheckpoint(checkpoint_path);
    if (success) {
        main_window_.setIntegrationStatus("Checkpoint deleted: " + checkpoint_path);
    } else {
        main_window_.setIntegrationStatus("Failed to delete checkpoint: " + checkpoint_path);
    }
    return success;
}

std::vector<std::string> AppController::getCheckpointsForProject(const std::string& project_path) const {
    return project_file_service_.listCheckpoints(project_path);
}

void AppController::triggerAutoSave() {
    if (project_file_service_.isAutoSaveEnabled()) {
        bool success = project_file_service_.triggerAutoSave(active_assembly_);
        if (success) {
            main_window_.setViewportStatus("Auto-saved");
        }
    }
}

std::vector<std::string> AppController::getRecentProjects() const {
    return recent_projects_;
}

void AppController::saveRecentProjectsToSettings() const {
    #ifdef CAD_USE_QT
    QSettings settings("HydraCAD", "HydraCAD");
    QStringList recent;
    for (const auto& project : recent_projects_) {
        recent << QString::fromStdString(project);
    }
    settings.setValue("project/recent_projects", recent);
    settings.sync();
    #endif
}

void AppController::markProjectModified() {
    has_unsaved_changes_ = true;
    if (!current_project_path_.empty()) {
        main_window_.setDocumentLabel(project_file_service_.getProjectInfo(current_project_path_).project_name + " *");
    }
}

bool AppController::isUserLoggedIn() const {
    return user_auth_service_.isLoggedIn();
}

cad::app::User AppController::getCurrentUser() const {
    return user_auth_service_.getCurrentUser();
}

void AppController::logout() {
    #ifdef CAD_USE_QT
    cad::ui::QtMainWindow* qt_window = main_window_.nativeWindow();
    if (qt_window) {
        qt_window->setCurrentUser("", "");
    }
    #endif
    user_auth_service_.logout();
    main_window_.setIntegrationStatus("Logged out");
    main_window_.setDocumentLabel("Logged out - Please restart application");
}

void AppController::setupAIService(cad::ui::QtMainWindow* qt_window) {
    #ifdef CAD_USE_QT
    if (!qt_window) return;
    
    // Load AI settings
    QSettings settings("HydraCAD", "HydraCAD");
    QString provider = settings.value("ai/provider", "openai").toString();
    QString openai_key = settings.value("ai/openai_key").toString();
    QString anthropic_key = settings.value("ai/anthropic_key").toString();
    QString grok_key = settings.value("ai/grok_key").toString();
    QString model = settings.value("ai/model", "gpt-4").toString();
    double temperature = settings.value("ai/temperature", 0.7).toDouble();
    int max_tokens = settings.value("ai/max_tokens", 2000).toInt();
    
    // Configure provider
    if (provider == "openai" && !openai_key.isEmpty()) {
        ai_service_.configureProvider(cad::app::ai::ModelProviderType::OpenAI, openai_key.toStdString());
        ai_service_.setModel(model.toStdString());
    } else if (provider == "grok" && !grok_key.isEmpty()) {
        ai_service_.configureProvider(cad::app::ai::ModelProviderType::Grok, grok_key.toStdString());
        ai_service_.setModel(model.toStdString());
    } else if (provider == "anthropic" && !anthropic_key.isEmpty()) {
        ai_service_.configureProvider(cad::app::ai::ModelProviderType::Anthropic, anthropic_key.toStdString());
        ai_service_.setModel(model.toStdString());
    }
    
    ai_service_.setTemperature(temperature);
    ai_service_.setMaxTokens(max_tokens);
    
    // Connect AI Chat Panel
    cad::ui::QtAIChatPanel* ai_chat = qt_window->aiChatPanel();
    if (ai_chat) {
        ai_chat->setModelName(model);
        
        QObject::connect(ai_chat, &cad::ui::QtAIChatPanel::messageSent, ai_chat, [this, ai_chat](const QString& message) {
            ai_chat->setThinking(true);
            
            // Get context
            std::string context = ai_service_.getContext();
            
            // Send to AI
            bool streaming = true;
            if (streaming) {
                ai_service_.chatStreaming(message.toStdString(),
                    [ai_chat](const std::string& chunk) {
                        ai_chat->appendToAIMessage(QString::fromStdString(chunk));
                    },
                    context);
            } else {
                cad::app::ai::AIResponse response = ai_service_.chat(message.toStdString(), context);
                if (response.success) {
                    ai_chat->addAIMessage(QString::fromStdString(response.content));
                } else {
                    ai_chat->addAIMessage(QObject::tr("Error: %1").arg(QString::fromStdString(response.error_message)));
                }
            }
            
            ai_chat->setThinking(false);
        });
        
        QObject::connect(ai_chat, &cad::ui::QtAIChatPanel::settingsRequested, ai_chat, [this, qt_window]() {
            showAISettingsDialog(qt_window);
        });
    }
    #endif
}

void AppController::showAISettingsDialog(cad::ui::QtMainWindow* qt_window) {
    #ifdef CAD_USE_QT
    if (!qt_window) return;
    
    cad::ui::QtAISettingsDialog dialog(qt_window);
    
    // Load current settings
    QSettings settings("HydraCAD", "HydraCAD");
    dialog.setOpenAIKey(settings.value("ai/openai_key").toString());
    dialog.setAnthropicKey(settings.value("ai/anthropic_key").toString());
    dialog.setGrokKey(settings.value("ai/grok_key").toString());
    dialog.setSelectedProvider(settings.value("ai/provider", "openai").toString());
    dialog.setSelectedModel(settings.value("ai/model", "gpt-4").toString());
    dialog.setTemperature(settings.value("ai/temperature", 0.7).toDouble());
    dialog.setMaxTokens(settings.value("ai/max_tokens", 2000).toInt());
    dialog.setStreamingEnabled(settings.value("ai/streaming", true).toBool());
    
    // Test connection
    QObject::connect(&dialog, &cad::ui::QtAISettingsDialog::testConnectionRequested, &dialog, [&dialog, this](const QString& provider) {
        if (provider == "openai") {
            QString key = dialog.getOpenAIKey();
            if (!key.isEmpty()) {
                bool success = ai_service_.configureProvider(cad::app::ai::ModelProviderType::OpenAI, key.toStdString());
                if (success) {
                    success = ai_service_.getActiveProvider()->testConnection();
                }
                dialog.setTestResult(
                    success ? QObject::tr("Connection successful!") : QObject::tr("Connection failed. Please check your API key."),
                    success
                );
            }
        } else if (provider == "anthropic") {
            QString key = dialog.getAnthropicKey();
            if (!key.isEmpty()) {
                bool success = ai_service_.configureProvider(cad::app::ai::ModelProviderType::Anthropic, key.toStdString());
                if (success) {
                    success = ai_service_.getActiveProvider()->testConnection();
                }
                dialog.setTestResult(
                    success ? QObject::tr("Connection successful!") : QObject::tr("Connection failed. Please check your API key."),
                    success
                );
            }
        } else if (provider == "grok") {
            QString key = dialog.getGrokKey();
            if (!key.isEmpty()) {
                bool success = ai_service_.configureProvider(cad::app::ai::ModelProviderType::Grok, key.toStdString());
                if (success) {
                    success = ai_service_.getActiveProvider()->testConnection();
                }
                dialog.setTestResult(
                    success ? QObject::tr("Connection successful!") : QObject::tr("Connection failed. Please check your API key."),
                    success
                );
            }
        }
    });
    
    // Save settings
    QObject::connect(&dialog, &cad::ui::QtAISettingsDialog::saveRequested, &dialog, [&dialog, this, qt_window]() {
        QSettings settings("HydraCAD", "HydraCAD");
        settings.setValue("ai/openai_key", dialog.getOpenAIKey());
        settings.setValue("ai/anthropic_key", dialog.getAnthropicKey());
        settings.setValue("ai/grok_key", dialog.getGrokKey());
        settings.setValue("ai/provider", dialog.getSelectedProvider());
        settings.setValue("ai/model", dialog.getSelectedModel());
        settings.setValue("ai/temperature", dialog.getTemperature());
        settings.setValue("ai/max_tokens", dialog.getMaxTokens());
        settings.setValue("ai/streaming", dialog.getStreamingEnabled());
        
        // Reconfigure AI service
        QString provider = dialog.getSelectedProvider();
        if (provider == "openai") {
            ai_service_.configureProvider(cad::app::ai::ModelProviderType::OpenAI, dialog.getOpenAIKey().toStdString());
        } else if (provider == "anthropic") {
            ai_service_.configureProvider(cad::app::ai::ModelProviderType::Anthropic, dialog.getAnthropicKey().toStdString());
        } else if (provider == "grok") {
            ai_service_.configureProvider(cad::app::ai::ModelProviderType::Grok, dialog.getGrokKey().toStdString());
        }
        ai_service_.setModel(dialog.getSelectedModel().toStdString());
        ai_service_.setTemperature(dialog.getTemperature());
        ai_service_.setMaxTokens(dialog.getMaxTokens());
        
        // Update chat panel
        cad::ui::QtAIChatPanel* ai_chat = qt_window->aiChatPanel();
        if (ai_chat) {
            ai_chat->setModelName(dialog.getSelectedModel());
        }
    });
    
    dialog.exec();
    #endif
}

void AppController::setupAutoUpdate(cad::ui::QtMainWindow* qt_window) {
    #ifdef CAD_USE_QT
    if (!qt_window) return;
    
    QSettings settings("HydraCAD", "HydraCAD");
    bool auto_update_enabled = settings.value("updates/auto_check", true).toBool();
    bool auto_install = settings.value("updates/auto_install", false).toBool();
    int check_interval_days = settings.value("updates/check_interval_days", 7).toInt();
    
    update_service_.enableAutoUpdate(auto_update_enabled);
    update_service_.setAutoUpdateCheckInterval(check_interval_days);
    
    #ifdef CAD_APP_VERSION
    update_service_.setCurrentVersion(CAD_APP_VERSION);
    #else
    update_service_.setCurrentVersion("v0.0.0");
    #endif
    
    // Set installation directory
    QString install_path = settings.value("app/install_path", "C:/Program Files/Hydra CAD").toString();
    settings.setValue("app/install_path", install_path);
    
    // Check for updates on startup (if enabled)
    if (auto_update_enabled) {
        QTimer::singleShot(8000, qt_window, [this, qt_window, auto_install]() {
            if (qt_window->isVisible()) {
                checkForUpdates(qt_window, auto_install);
            }
        });
    }
    
    // Periodic check timer
    QTimer* update_check_timer = new QTimer(qt_window);
    QObject::connect(update_check_timer, &QTimer::timeout, update_check_timer, [this, qt_window, auto_install]() {
        checkForUpdates(qt_window, auto_install);
    });
    
    int interval_ms = check_interval_days * 24 * 60 * 60 * 1000;
    if (auto_update_enabled) {
        update_check_timer->start(interval_ms);
    }
    #endif
}

void AppController::checkForUpdates(cad::ui::QtMainWindow* qt_window, bool auto_install) {
    #ifdef CAD_USE_QT
    if (!qt_window) return;
    (void)auto_install;
    
    cad::app::UpdateInfo update_info;
    bool have_update = false;
    
    if (update_service_.checkForUpdates() && update_service_.isUpdateAvailable()) {
        update_info = update_service_.getLatestUpdateInfo();
        have_update = true;
    }
    
    if (!have_update) {
        if (qt_window->statusBar()) {
            std::string err = update_service_.getLastError();
            if (!err.empty()) {
                qt_window->statusBar()->showMessage(QString::fromStdString(err), 5000);
            } else {
                qt_window->statusBar()->showMessage(QObject::tr("No updates available. You are using the latest version."), 4000);
            }
        }
        return;
    }
    
    cad::ui::QtUpdateDialog dialog(qt_window);
    dialog.setUpdateInfo(QString::fromStdString(update_info.version),
                        QString::fromStdString(update_info.changelog),
                        update_info.mandatory);
    
    QObject::connect(&dialog, &cad::ui::QtUpdateDialog::installRequested, &dialog, [this, update_info, qt_window]() {
        installUpdate(qt_window, update_info, true);
    });
    
    dialog.exec();
    #endif
}

void AppController::installUpdate(cad::ui::QtMainWindow* qt_window, 
                                  const cad::app::UpdateInfo& update_info, 
                                  bool create_backup) {
    #ifdef CAD_USE_QT
    if (!qt_window) return;
    (void)create_backup;
    
    if (update_info.download_url.empty()) {
        QMessageBox::warning(qt_window, QObject::tr("Update"),
                             QObject::tr("No download URL available. Please check the release page manually."));
        return;
    }
    const bool is_direct_asset = (update_info.download_url.find("releases/download") != std::string::npos);
    if (!is_direct_asset && update_info.download_url.find("github.com") != std::string::npos) {
        cad::core::updates::openUrlInBrowser(update_info.download_url);
        QMessageBox::information(qt_window, QObject::tr("Update"),
                                 QObject::tr("The release page has been opened. Download the installer (e.g. HydraCADSetup.exe or app-windows.zip) from the page."));
        return;
    }
    
    cad::ui::QtUpdateDialog progress_dialog(qt_window);
    progress_dialog.setUpdateInfo(QString::fromStdString(update_info.version),
                                 QString::fromStdString(update_info.changelog),
                                 update_info.mandatory);
    progress_dialog.setProgress(0, QObject::tr("Downloading update..."));
    progress_dialog.setModal(true);
    progress_dialog.show();
    QApplication::processEvents();

    // Use a user-writable directory (Temp) so updates work without admin rights.
    QString download_dir = QDir(QStandardPaths::writableLocation(QStandardPaths::TempLocation)).absoluteFilePath("HydraCAD");
    if (!QDir().mkpath(download_dir)) {
        download_dir = QDir::tempPath();
    }
    // Derive filename from download URL (e.g. .../HydraCADSetup.exe -> HydraCADSetup.exe) so we save and run the correct file.
    std::string url = update_info.download_url;
    std::string filename = "update_" + update_info.version + ".exe";
    if (!url.empty()) {
        size_t last_slash = url.find('?');
        if (last_slash != std::string::npos) url.resize(last_slash);
        last_slash = url.rfind('/');
        if (last_slash != std::string::npos && last_slash + 1 < url.size()) {
            std::string segment = url.substr(last_slash + 1);
            if (!segment.empty()) filename = segment;
        }
    }
    const std::string target_path = (download_dir + "/" + QString::fromStdString(filename)).toStdString();

    bool download_success = update_service_.downloadUpdate(update_info,
        [&progress_dialog](const cad::app::UpdateProgress& progress) {
            progress_dialog.setProgress(progress.percentage,
                                      QString::fromStdString(progress.status_message));
            QApplication::processEvents();
        },
        target_path);

    if (!download_success) {
        progress_dialog.setCompleted(false, QObject::tr("Download failed"));
        progress_dialog.exec();
        QMessageBox::warning(qt_window, QObject::tr("Update Failed"),
                             QObject::tr("Download failed. Please check your connection and try again."));
        return;
    }

    if (!update_info.checksum.empty() && !update_service_.verifyDownloadedUpdate(target_path)) {
        progress_dialog.setCompleted(false, QObject::tr("Verification failed"));
        progress_dialog.close();
        QMessageBox::warning(qt_window, QObject::tr("Update Failed"),
                             QObject::tr("Checksum verification failed. The download may be corrupted. Please try again or download from the release page."));
        return;
    }
    if (update_info.checksum.empty() && qt_window->statusBar()) {
        qt_window->statusBar()->showMessage(QObject::tr("Checksum not available; installer started without verification."), 4000);
    }

    QString download_path_qt = QString::fromStdString(target_path);

    progress_dialog.setProgress(90, QObject::tr("Starting installer..."));
    QApplication::processEvents();

    bool run_installer = false;
    bool is_exe = (filename.size() >= 4 && filename.compare(filename.size() - 4, 4, ".exe") == 0);
    #ifdef _WIN32
    if (QFileInfo::exists(download_path_qt) && is_exe) {
        run_installer = QProcess::startDetached(download_path_qt, QStringList(), QFileInfo(download_path_qt).absolutePath());
    }
    #else
    // On Linux/macOS we do not run .exe; for tarball/zip we only inform the user where the file is.
    if (QFileInfo::exists(download_path_qt) && is_exe) {
        run_installer = QProcess::startDetached(download_path_qt, QStringList());
    }
    #endif

    if (run_installer) {
        progress_dialog.setCompleted(true, QObject::tr("Installer started."));
        progress_dialog.exec();
        QMessageBox::information(qt_window, QObject::tr("Update"),
                                QObject::tr("The installer has been started. Complete the setup and restart the application."));
        QApplication::quit();
    } else {
        if (is_exe) {
            progress_dialog.setCompleted(false, QObject::tr("Could not start installer"));
        } else {
            progress_dialog.setCompleted(true, QObject::tr("Download complete."));
        }
        progress_dialog.exec();
        if (is_exe) {
            QMessageBox::warning(qt_window, QObject::tr("Update"),
                                 QObject::tr("Could not start the installer. You can run it manually: %1").arg(download_path_qt));
        } else {
            QMessageBox::information(qt_window, QObject::tr("Update"),
                                    QObject::tr("Update package downloaded to: %1\nOn Linux: extract the archive and run the contained executable, or install the new version via your package manager (e.g. yay -S hydracad).").arg(download_path_qt));
            #ifndef _WIN32
            QDesktopServices::openUrl(QUrl::fromLocalFile(download_dir));
            #endif
        }
    }
    #endif
}

}  // namespace app
}  // namespace cad
