#include <QApplication>
#include <QFile>
#include <QSettings>
#include <QTranslator>
#include <QCoreApplication>
#include <QIcon>
#include <QColor>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QPainter>
#include <QPixmap>
#include <QSplashScreen>
#include <QByteArray>
#include <QStandardPaths>
#include <QTextStream>
#include <QMutex>
#include <QMutexLocker>

#include <filesystem>
#include <string>
#include <exception>
#include <cstdlib>
#include <cstdio>

#ifdef __linux__
#include <unistd.h>
#include <stdlib.h>  /* setenv, getenv (POSIX) */
#endif
#ifdef _WIN32
#include <windows.h>
#endif

#include "app/AppController.h"
#include "app/CADApplication.h"
#include "ui/qt/QtMainWindow.h"
#include "ui/theme/ThemeManager.h"

namespace {
QFile* g_log_file = nullptr;
QMutex g_log_mutex;
QString g_marker_path;
QString g_crash_log_path;

void logMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& message) {
    QMutexLocker locker(&g_log_mutex);
    if (!g_log_file || !g_log_file->isOpen()) {
        return;
    }
    const QString timestamp = QDateTime::currentDateTime().toString(Qt::ISODate);
    const char* type_label = "INFO";
    switch (type) {
        case QtDebugMsg: type_label = "DEBUG"; break;
        case QtInfoMsg: type_label = "INFO"; break;
        case QtWarningMsg: type_label = "WARN"; break;
        case QtCriticalMsg: type_label = "CRITICAL"; break;
        case QtFatalMsg: type_label = "FATAL"; break;
    }
    QTextStream out(g_log_file);
    out << timestamp << " [" << type_label << "] " << message;
    if (context.file) {
        out << " (" << context.file << ":" << context.line << ")";
    }
    out << "\n";
    out.flush();
}

void setupStartupLogging() {
    const QString app_data = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (app_data.isEmpty()) {
        return;
    }
    QDir dir(app_data);
    if (!dir.exists("logs")) {
        dir.mkpath("logs");
    }
    const QString log_path = dir.filePath("logs/startup.log");
    g_log_file = new QFile(log_path);
    if (!g_log_file->open(QIODevice::Append | QIODevice::Text)) {
        delete g_log_file;
        g_log_file = nullptr;
        return;
    }
    qInstallMessageHandler(logMessageHandler);
    qInfo() << "Hydra CAD startup";

    g_crash_log_path = dir.filePath("logs/last_crash.log");
    g_marker_path = dir.filePath("logs/last_session.marker");
    if (!g_marker_path.isEmpty() && QFile::exists(g_marker_path)) {
        qWarning() << "Previous session ended unexpectedly.";
    }
    QFile marker_file(g_marker_path);
    if (marker_file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        QTextStream out(&marker_file);
        out << QDateTime::currentDateTime().toString(Qt::ISODate) << "\n";
    }
}

void writeCrashLog(const QString& message) {
    if (g_crash_log_path.isEmpty()) {
        return;
    }
    QFile crash_file(g_crash_log_path);
    if (!crash_file.open(QIODevice::Append | QIODevice::Text)) {
        return;
    }
    QTextStream out(&crash_file);
    out << QDateTime::currentDateTime().toString(Qt::ISODate) << " " << message << "\n";
    out.flush();
}

#ifdef _WIN32
bool preflightCheckWindows(const std::filesystem::path& exe_dir) {
    auto exe_path = exe_dir / "cad_desktop.exe";
    auto qt_gui = exe_dir / "Qt6Gui.dll";
    auto qwindows = exe_dir / "platforms" / "qwindows.dll";
    if (!std::filesystem::exists(exe_path) ||
        !std::filesystem::exists(qt_gui) ||
        !std::filesystem::exists(qwindows)) {
        MessageBoxA(nullptr,
                    "Hydra CAD could not start because required runtime files are missing.\n"
                    "Please re-install the application.",
                    "Hydra CAD",
                    MB_OK | MB_ICONERROR);
        return false;
    }
    return true;
}
#endif

#ifdef __linux__
// Resolve real executable path so the app works when started from desktop/launcher
// (argv[0] may be just "cad_desktop" or a path relative to wrong CWD).
std::filesystem::path getLinuxExeDir() {
    char buf[4096];
    ssize_t n = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    if (n <= 0) return {};
    buf[n] = '\0';
    std::filesystem::path exe_path(buf);
    return exe_path.parent_path();
}
#endif
}  // namespace

int main(int argc, char** argv) {
#ifdef __linux__
    (void)write(2, "Hydra CAD: main() start\n", 23);
    (void)write(2, "Hydra CAD: before path\n", 22);
#endif
    std::filesystem::path exe_dir;
#ifdef __linux__
    (void)write(2, "Hydra CAD: after path\n", 21);
#endif
    try {
#ifdef __linux__
        (void)write(2, "Hydra CAD: in try, getLinuxExeDir\n", 34);
#endif
#ifdef __linux__
        exe_dir = getLinuxExeDir();
        fprintf(stderr, "Hydra CAD: exe_dir from /proc/self/exe = %s\n", exe_dir.empty() ? "(empty)" : exe_dir.string().c_str());
        fflush(stderr);
        if (exe_dir.empty())
#endif
        {
            std::filesystem::path exe_path = std::filesystem::absolute(argv[0]);
            exe_dir = exe_path.parent_path();
            fprintf(stderr, "Hydra CAD: exe_dir from argv[0] = %s\n", exe_dir.string().c_str());
            fflush(stderr);
        }
        const std::filesystem::path platform_path = exe_dir / "platforms";
        // Only override Qt plugin paths when we ship our own (e.g. portable/Windows).
        if (std::filesystem::is_directory(platform_path)) {
            const std::string plugin_root = exe_dir.string();
            const std::string platform_path_env = platform_path.string();
            setenv("QT_PLUGIN_PATH", plugin_root.c_str(), 1);
            setenv("QT_QPA_PLATFORM_PLUGIN_PATH", platform_path_env.c_str(), 1);
        }
    } catch (const std::exception& e) {
        fprintf(stderr, "Hydra CAD: exception in path setup: %s\n", e.what());
        fflush(stderr);
    } catch (...) {
        fprintf(stderr, "Hydra CAD: unknown exception in path setup\n");
        fflush(stderr);
    }

    fprintf(stderr, "Hydra CAD: before Wayland check\n");
    fflush(stderr);
#ifdef __linux__
    // SoQt/Coin3D often crash or fail to create OpenGL context on native Wayland.
    if (!getenv("QT_QPA_PLATFORM") && getenv("WAYLAND_DISPLAY")) {
        setenv("QT_QPA_PLATFORM", "xcb", 1);
    }
#endif
    fprintf(stderr, "Hydra CAD: before QApplication\n");
    fflush(stderr);

#ifdef _WIN32
    if (!exe_dir.empty() && !preflightCheckWindows(exe_dir)) {
        return 1;
    }
#endif

    fprintf(stderr, "Hydra CAD: before QApplication\n");
    fflush(stderr);
    QApplication qt_app(argc, argv);
    fprintf(stderr, "Hydra CAD: after QApplication, CWD=%s\n", QDir::currentPath().toUtf8().constData());
    fflush(stderr);
    // Ensure working directory is the application directory so plugins, resources and
    // relative paths resolve correctly regardless of how the app was started (shortcut,
    // file manager, launcher with argv[0] just the program name, etc.).
    if (!exe_dir.empty()) {
        QDir::setCurrent(QString::fromStdString(exe_dir.string()));
    } else {
        QDir::setCurrent(QCoreApplication::applicationDirPath());
    }
    setupStartupLogging();

    std::set_terminate([]() {
        QString message = "Unhandled exception";
        std::exception_ptr eptr = std::current_exception();
        if (eptr) {
            try {
                std::rethrow_exception(eptr);
            } catch (const std::exception& ex) {
                message = QString("Unhandled exception: %1").arg(ex.what());
            } catch (...) {
                message = "Unhandled exception: unknown";
            }
        }
        fprintf(stderr, "Hydra CAD FATAL: %s\n", message.toUtf8().constData());
        fflush(stderr);
        writeCrashLog(message);
        std::_Exit(1);
    });

    QObject::connect(&qt_app, &QCoreApplication::aboutToQuit, []() {
        if (!g_marker_path.isEmpty()) {
            QFile::remove(g_marker_path);
        }
        if (g_log_file) {
            g_log_file->flush();
            g_log_file->close();
        }
    });

    QTranslator translator;
    QSettings settings("HydraCAD", "HydraCAD");
    const QString language_code = settings.value("ui/language", "en").toString();
    if (language_code != "en") {
        const QString resource_path = QString(":/i18n/hydracad_%1.qm").arg(language_code);
        const QString file_path = QCoreApplication::applicationDirPath() + "/i18n/hydracad_" + language_code + ".qm";
        if (translator.load(resource_path) || translator.load(file_path)) {
            qt_app.installTranslator(&translator);
        }
    }
    qt_app.setWindowIcon(QIcon(":/icons/app/app_icon.png"));

    cad::app::CADApplication application;
    cad::app::AppController controller;
    application.setController(&controller);
    
    fprintf(stderr, "Hydra CAD: before initializeWithLogin\n");
    fflush(stderr);
    // Require login before initializing
    if (!controller.initializeWithLogin()) {
        fprintf(stderr, "Hydra CAD: initializeWithLogin returned false, exiting\n");
        fflush(stderr);
        return 1;  // User cancelled login
    }
    fprintf(stderr, "Hydra CAD: after initializeWithLogin\n");
    fflush(stderr);

    // Show a simple loading screen while initializing
    QPixmap splash_pixmap(480, 260);
    splash_pixmap.fill(QColor(20, 20, 20));
    QPainter painter(&splash_pixmap);
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 20, QFont::Bold));
    painter.drawText(splash_pixmap.rect().adjusted(0, -20, 0, -20), Qt::AlignCenter, "Hydra CAD");
    painter.setFont(QFont("Arial", 12));
    painter.drawText(splash_pixmap.rect().adjusted(0, 50, 0, 0), Qt::AlignCenter, "Loading...");
    painter.end();

    QSplashScreen splash(splash_pixmap);
    splash.setWindowFlag(Qt::WindowStaysOnTopHint, true);
    splash.show();
    splash.showMessage("Initializing...", Qt::AlignHCenter | Qt::AlignBottom, Qt::white);
    qt_app.processEvents();

    // Apply modern theme
    #ifdef CAD_USE_QT
    cad::ui::ThemeManager& theme = cad::ui::ThemeManager::instance();
    qt_app.setStyleSheet(theme.generateStylesheet());
    #else
    QFile styleFile(":/styles/InventorLight.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        qt_app.setStyleSheet(styleFile.readAll());
    }
    #endif

    controller.initialize();
    splash.showMessage("Opening window...", Qt::AlignHCenter | Qt::AlignBottom, Qt::white);
    qt_app.processEvents();

    cad::ui::MainWindow& main_window = controller.mainWindow();
#ifdef CAD_USE_QT
    if (main_window.hasNativeWindow()) {
        QWidget* win = main_window.nativeWindow();
        win->show();
        win->raise();
        win->activateWindow();
    }
#endif
    splash.finish(main_window.hasNativeWindow() ? main_window.nativeWindow() : nullptr);
#ifdef CAD_USE_QT
    // Ensure main window stays visible after splash (e.g. under XWayland)
    if (main_window.hasNativeWindow()) {
        QWidget* win = main_window.nativeWindow();
        win->raise();
        win->activateWindow();
        qt_app.processEvents();
    }
#endif

    int result = qt_app.exec();
    application.shutdown();
    return result;
}
