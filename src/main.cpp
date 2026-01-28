#include <QApplication>
#include <QFile>
#include <QSettings>
#include <QTranslator>
#include <QCoreApplication>
#include <QIcon>

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

    cad::ui::MainWindow& main_window = controller.mainWindow();
#ifdef CAD_USE_QT
    if (main_window.hasNativeWindow()) {
        main_window.nativeWindow()->show();
    }
#endif

    int result = qt_app.exec();
    application.shutdown();
    return result;
}
