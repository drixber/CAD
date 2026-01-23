#include <QApplication>
#include <QFile>

#include "app/AppController.h"
#include "app/CADApplication.h"
#include "ui/qt/QtMainWindow.h"

int main(int argc, char** argv) {
    QApplication qt_app(argc, argv);

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
