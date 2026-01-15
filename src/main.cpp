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
    application.initialize();

    QFile styleFile(":/styles/InventorLight.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        qt_app.setStyleSheet(styleFile.readAll());
    }

    cad::ui::MainWindow& main_window = controller.mainWindow();
#ifdef CAD_USE_QT
    if (main_window.hasNativeWindow()) {
        main_window.nativeWindow()->show();
    }
#endif
#ifdef CAD_USE_QT
    if (main_window.hasNativeWindow()) {
        main_window.nativeWindow()->show();
    }
#endif

    int result = qt_app.exec();
    application.shutdown();
    return result;
}
