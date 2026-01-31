#include "AppController.h"
#include "ai/AIService.h"
#include "UpdateInstaller.h"
#include "HttpClient.h"
#include "core/updates/UpdateChecker.h"
#include <sstream>
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
#include <QProcess>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#endif

namespace cad {
namespace app {

AppController::AppController() = default;

bool AppController::initializeWithLogin() {
    #ifdef CAD_USE_QT
    // Check if user is already logged in (remember me)
    if (user_auth_service_.loadSavedSession()) {
        QSettings settings("HydraCAD", "HydraCAD");
        QString remembered_username = settings.value("auth/remember_username").toString();
        if (!remembered_username.isEmpty()) {
            // Show login dialog with remembered username
            cad::ui::QtLoginDialog login_dialog;
            login_dialog.setRememberedUsername(remembered_username);
            
            bool login_success = false;
            QObject::connect(&login_dialog, &cad::ui::QtLoginDialog::loginRequested, &login_dialog,
                   [this, &login_success, &login_dialog](const QString& username, const QString& password, bool remember) {
                cad::app::LoginResult result = user_auth_service_.login(username.toStdString(), password.toStdString());
                if (result.success) {
                    login_success = true;
                    if (remember) {
                        user_auth_service_.saveSession(username.toStdString(), true);
                    }
                    login_dialog.accept();
                } else {
                    QMessageBox::warning(&login_dialog, QObject::tr("Login Failed"), 
                                       QString::fromStdString(result.error_message));
                }
            });
            
            QObject::connect(&login_dialog, &cad::ui::QtLoginDialog::registerRequested, &login_dialog, [this, &login_dialog]() {
                login_dialog.hide();
                cad::ui::QtRegisterDialog register_dialog;
                
                QObject::connect(&register_dialog, &cad::ui::QtRegisterDialog::registerRequested, &register_dialog,
                       [this, &register_dialog](const QString& username, const QString& email,
                                               const QString& password, const QString& confirm) {
                    if (password != confirm) {
                        register_dialog.setError(QObject::tr("Passwords do not match"));
                        return;
                    }
                    
                    cad::app::RegisterResult result = user_auth_service_.registerUser(
                        username.toStdString(), email.toStdString(), password.toStdString());
                    
                    if (result.success) {
                        QMessageBox::information(&register_dialog, QObject::tr("Registration Successful"),
                                               QObject::tr("Account created successfully. You can now login."));
                        register_dialog.accept();
                    } else {
                        register_dialog.setError(QString::fromStdString(result.error_message));
                    }
                });
                
                if (register_dialog.exec() == QDialog::Accepted) {
                    login_dialog.show();
                }
            });
            
            if (login_dialog.exec() != QDialog::Accepted || !login_success) {
                return false;
            }
        } else {
            return requireLogin();
        }
    } else {
        return requireLogin();
    }
    #else
    // Non-Qt: Simple login check
    return requireLogin();
    #endif
    
    return true;
}

bool AppController::requireLogin() {
    #ifdef CAD_USE_QT
    cad::ui::QtLoginDialog login_dialog;
    bool login_success = false;
    
    QObject::connect(&login_dialog, &cad::ui::QtLoginDialog::loginRequested, &login_dialog,
           [this, &login_success, &login_dialog](const QString& username, const QString& password, bool remember) {
        cad::app::LoginResult result = user_auth_service_.login(username.toStdString(), password.toStdString());
        if (result.success) {
            login_success = true;
            if (remember) {
                user_auth_service_.saveSession(username.toStdString(), true);
            }
            login_dialog.accept();
        } else {
            login_dialog.setError(QString::fromStdString(result.error_message));
        }
    });
    
    QObject::connect(&login_dialog, &cad::ui::QtLoginDialog::registerRequested, &login_dialog, [this, &login_dialog]() {
        login_dialog.hide();
        cad::ui::QtRegisterDialog register_dialog;
        
        QObject::connect(&register_dialog, &cad::ui::QtRegisterDialog::registerRequested, &register_dialog,
               [this, &register_dialog](const QString& username, const QString& email,
                                       const QString& password, const QString& confirm) {
            if (password != confirm) {
                register_dialog.setError(QObject::tr("Passwords do not match"));
                return;
            }
            
            cad::app::RegisterResult result = user_auth_service_.registerUser(
                username.toStdString(), email.toStdString(), password.toStdString());
            
            if (result.success) {
                QMessageBox::information(&register_dialog, QObject::tr("Registration Successful"),
                                       QObject::tr("Account created successfully. You can now login."));
                register_dialog.accept();
            } else {
                register_dialog.setError(QString::fromStdString(result.error_message));
            }
        });
        
        if (register_dialog.exec() == QDialog::Accepted) {
            login_dialog.show();
        }
    });
    
    return login_dialog.exec() == QDialog::Accepted && login_success;
    #else
    // Non-Qt: Always allow (no authentication)
    return true;
    #endif
}

void AppController::initialize() {
    main_window_.initializeLayout();
    main_window_.setViewportStatus("3D viewport ready");
    main_window_.setWorkspaceMode("General");
    main_window_.setDocumentLabel("MainDocument");
    if (freecad_.initializeSession()) {
        freecad_.createDocument("MainDocument");
        main_window_.setIntegrationStatus("FreeCAD on");
    } else {
        main_window_.setIntegrationStatus("FreeCAD off");
    }
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
    if (freecad_.isAvailable()) {
        freecad_.syncSketch(active_sketch_);
        freecad_.syncConstraints(active_sketch_);
        freecad_.syncGeometry(active_sketch_);
    }
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
        
        // Setup user management
        cad::app::User current_user = user_auth_service_.getCurrentUser();
        if (!current_user.username.empty()) {
            qt_window->setCurrentUser(current_user.username, current_user.email);
        }
        qt_window->setLogoutHandler([this]() {
            logout();
        });
        qt_window->setProfileHandler([this, qt_window]() {
            cad::app::User u = user_auth_service_.getCurrentUser();
            if (u.username.empty()) {
                QMessageBox::information(qt_window, QObject::tr("Profile"), QObject::tr("Not logged in."));
                return;
            }
            QString text = QObject::tr("User: %1").arg(QString::fromStdString(u.username));
            if (!u.email.empty()) {
                text += QLatin1String("\n") + QObject::tr("Email: %1").arg(QString::fromStdString(u.email));
            }
            QMessageBox::information(qt_window, QObject::tr("Profile"), text);
        });

        // AI Service integration
        setupAIService(qt_window);
        
        qt_window->setCheckForUpdatesHandler([this, qt_window]() {
            checkForUpdates(qt_window, false);
        });
        setupAutoUpdate(qt_window);

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
}

cad::ui::MainWindow& AppController::mainWindow() {
    return main_window_;
}

cad::core::Modeler& AppController::modeler() {
    return modeler_;
}

cad::core::FreeCADAdapter& AppController::freecad() {
    return freecad_;
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

void AppController::executeCommand(const std::string& command) {
    if (command == "Parameters") {
        main_window_.setParameterSummary(buildParameterSummary(active_sketch_));
        main_window_.setParameterCount(static_cast<int>(active_sketch_.parameters().size()));
        main_window_.setViewportStatus("Parameters: " + std::to_string(active_sketch_.parameters().size()) + " items");
    } else if (command == "Flange" || command == "Bend" || command == "Unfold" || command == "Refold") {
        cad::modules::SheetMetalRequest request;
        request.targetPart = "Bracket";
        request.operation = cad::modules::SheetMetalOperation::Flange;
        cad::modules::SheetMetalResult result = sheet_metal_service_.applyOperation(request);
        main_window_.setIntegrationStatus(result.message);
        main_window_.setViewportStatus("Sheet metal command queued");
    } else if (command == "Rectangular Pattern" || command == "Circular Pattern" || command == "Curve Pattern") {
        cad::modules::PatternRequest request;
        request.targetFeature = "Hole1";
        request.type = cad::modules::PatternType::Rectangular;
        request.instanceCount = 6;
        cad::modules::PatternResult result = pattern_service_.createPattern(request);
        main_window_.setIntegrationStatus(result.message);
        main_window_.setViewportStatus("Pattern command queued");
    } else if (command == "Direct Edit" || command == "Freeform") {
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
        cad::modules::RoutingResult result = routing_service_.createRoute(request);
        main_window_.setIntegrationStatus(result.message);
        main_window_.setViewportStatus("Routing command queued");
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
        main_window_.setViewportStatus("Simulation queued");
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
        cad::modules::DrawingResult result = drawing_service_.createDrawing(request);
        if (result.success) {
            if (freecad_.isAvailable()) {
                freecad_.createDrawing(result.drawingId, "A4_Landscape");
            }
            cad::drawings::DrawingDocument document =
                drawing_service_.buildDocumentSkeleton(result.drawingId);
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
            main_window_.setViewportStatus("Drawing view created");
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
        cad::modules::DrawingResult result = drawing_service_.createDrawing(request);
        if (result.success) {
            cad::drawings::DrawingDocument document =
                drawing_service_.buildDocumentSkeleton(result.drawingId);
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
        cad::modules::DrawingResult result = drawing_service_.createDrawing(request);
        if (result.success) {
            cad::drawings::DrawingDocument document =
                drawing_service_.buildDocumentSkeleton(result.drawingId);
            main_window_.setIntegrationStatus("Section view created");
            main_window_.setViewportStatus("Section view: " + result.drawingId);
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
                QStringList presets = {"Default", "ISO", "ANSI", "JIS"};
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
    } else if (command == "Mate") {
        main_window_.setMateCount(static_cast<int>(active_assembly_.mates().size()));
        main_window_.setViewportStatus("Mate command ready");
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
    } else if (command == "Line") {
        // Add a line to the active sketch
        cad::core::Point2D start{0.0, 0.0};
        cad::core::Point2D end{50.0, 50.0};
        std::string geom_id = active_sketch_.addLine(start, end);
        main_window_.setIntegrationStatus("Line added: " + geom_id);
        main_window_.setViewportStatus("Line created");
        
        // Render in viewport
        #ifdef CAD_USE_QT
        cad::ui::QtMainWindow* qt_window = main_window_.nativeWindow();
        if (qt_window && qt_window->viewport3D()) {
            qt_window->viewport3D()->renderGeometry(geom_id, nullptr);
        }
        #endif
    } else if (command == "Rectangle") {
        // Add a rectangle to the active sketch
        cad::core::Point2D corner{0.0, 0.0};
        double width = 100.0;
        double height = 50.0;
        std::string geom_id = active_sketch_.addRectangle(corner, width, height);
        main_window_.setIntegrationStatus("Rectangle added: " + geom_id);
        main_window_.setViewportStatus("Rectangle created");
        
        // Render in viewport
        #ifdef CAD_USE_QT
        cad::ui::QtMainWindow* qt_window = main_window_.nativeWindow();
        if (qt_window && qt_window->viewport3D()) {
            qt_window->viewport3D()->renderGeometry(geom_id, nullptr);
        }
        #endif
    } else if (command == "Circle") {
        // Add a circle to the active sketch
        cad::core::Point2D center{25.0, 25.0};
        double radius = 20.0;
        std::string geom_id = active_sketch_.addCircle(center, radius);
        main_window_.setIntegrationStatus("Circle added: " + geom_id);
        main_window_.setViewportStatus("Circle created");
        
        // Render in viewport
        #ifdef CAD_USE_QT
        cad::ui::QtMainWindow* qt_window = main_window_.nativeWindow();
        if (qt_window && qt_window->viewport3D()) {
            qt_window->viewport3D()->renderGeometry(geom_id, nullptr);
        }
        #endif
    } else if (command == "Arc") {
        // Add an arc to the active sketch
        cad::core::Point2D center{25.0, 25.0};
        double radius = 20.0;
        double start_angle = 0.0;
        double end_angle = 90.0;
        std::string geom_id = active_sketch_.addArc(center, radius, start_angle, end_angle);
        main_window_.setIntegrationStatus("Arc added: " + geom_id);
        main_window_.setViewportStatus("Arc created");
        
        // Render in viewport
        #ifdef CAD_USE_QT
        cad::ui::QtMainWindow* qt_window = main_window_.nativeWindow();
        if (qt_window && qt_window->viewport3D()) {
            qt_window->viewport3D()->renderGeometry(geom_id, nullptr);
        }
        #endif
    } else if (command == "Constraint") {
        main_window_.setConstraintCount(static_cast<int>(active_sketch_.constraints().size()));
        main_window_.setViewportStatus("Constraints: " + std::to_string(active_sketch_.constraints().size()) + " items");
    } else if (command == "Extrude") {
        // Create extrude feature from active sketch
        if (!active_sketch_.geometry().empty()) {
            cad::core::Part part = modeler_.createPart(active_sketch_);
            double depth = 10.0;  // Default depth
            // Try to parse depth from command if available
            modeler_.applyExtrude(part, active_sketch_.name(), depth);
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
    } else if (command == "Revolve") {
        // Create revolve feature from active sketch
        if (!active_sketch_.geometry().empty()) {
            cad::core::Part part = modeler_.createPart(active_sketch_);
            double angle = 360.0;  // Default full revolution
            modeler_.applyRevolve(part, active_sketch_.name(), angle);
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
        modeler_.applyHole(part, diameter, depth);
        main_window_.setIntegrationStatus("Hole created: diameter=" + std::to_string(diameter));
        main_window_.setViewportStatus("Hole feature applied");
        
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
        } else {
            main_window_.setIntegrationStatus("Angle: Need at least 2 components");
            main_window_.setViewportStatus("Add components first");
        }
    } else if (command == "Pattern") {
        main_window_.setIntegrationStatus("Pattern command ready");
        main_window_.setViewportStatus("Assembly pattern command active");
    } else if (command == "Visibility" || command == "Appearance" || command == "Environment") {
        main_window_.setIntegrationStatus("View: " + command);
        main_window_.setViewportStatus("View " + command + " command active");
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
        }
        has_unsaved_changes_ = false;
        current_project_path_ = file_path;
        ProjectFileInfo info = project_file_service_.getProjectInfo(file_path);
        main_window_.setIntegrationStatus("Project loaded: " + file_path);
        main_window_.setDocumentLabel(info.project_name);
        
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
        std::string current_tag = update_service_.getCurrentVersion();
        if (current_tag.empty()) {
            current_tag = "v0.0.0";
        }
        cad::core::updates::UpdateInfo gh;
        #ifdef CAD_USE_QT_NETWORK
        const std::string api_url = "https://api.github.com/repos/drixber/CAD/releases/latest";
        cad::app::HttpResponse api_resp = update_service_.getHttpClient().get(api_url, {{"Accept", "application/vnd.github.v3+json"}});
        if (api_resp.success && !api_resp.body.empty()) {
            gh = cad::core::updates::parseGithubReleaseResponse(api_resp.body, current_tag);
        } else {
            gh = cad::core::updates::checkGithubLatestRelease("drixber", "CAD", current_tag);
        }
        #else
        gh = cad::core::updates::checkGithubLatestRelease("drixber", "CAD", current_tag);
        #endif
        if (gh.updateAvailable && !gh.releaseUrl.empty()) {
            update_info.version = gh.latestTag;
            update_info.changelog = !gh.body.empty() ? gh.body : QObject::tr("See release page for changelog.").toStdString();
            update_info.download_url = !gh.assetDownloadUrl.empty() ? gh.assetDownloadUrl : gh.releaseUrl;
            update_info.mandatory = false;
            have_update = true;
        } else {
            if (qt_window->statusBar()) {
                if (!gh.error.empty()) {
                    qt_window->statusBar()->showMessage(QString::fromStdString(gh.error), 5000);
                } else {
                    qt_window->statusBar()->showMessage(QObject::tr("No updates available. You are using the latest version."), 4000);
                }
            }
            return;
        }
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
        progress_dialog.setCompleted(true, QObject::tr("Download complete."));
        progress_dialog.exec();
        if (is_exe) {
            QMessageBox::warning(qt_window, QObject::tr("Update"),
                                 QObject::tr("Could not start the installer. You can run it manually: %1").arg(download_path_qt));
        } else {
            QMessageBox::information(qt_window, QObject::tr("Update"),
                                    QObject::tr("Update package downloaded to: %1\nPlease extract or run the installer manually.").arg(download_path_qt));
        }
    }
    #endif
}

}  // namespace app
}  // namespace cad
