#include <QApplication>
#include <QFile>
#include <QSettings>
#include <QTranslator>
#include <QCoreApplication>
#include <QIcon>
#include <QColor>
#include <QPainter>
#include <QPixmap>
#include <QSplashScreen>

#include "app/AppController.h"
#include "app/CADApplication.h"
#include "ui/qt/QtMainWindow.h"
#include "ui/theme/ThemeManager.h"

int main(int argc, char** argv) {
    QApplication qt_app(argc, argv);

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
    
    // Require login before initializing
    if (!controller.initializeWithLogin()) {
        return 1;  // User cancelled login
    }

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

    cad::ui::MainWindow& main_window = controller.mainWindow();
#ifdef CAD_USE_QT
    if (main_window.hasNativeWindow()) {
        main_window.nativeWindow()->show();
    }
#endif
    splash.finish(main_window.hasNativeWindow() ? main_window.nativeWindow() : nullptr);

    int result = qt_app.exec();
    application.shutdown();
    return result;
}
