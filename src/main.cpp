#include <QApplication>

#include "app/AppController.h"
#include "app/CADApplication.h"
#include "ui/qt/QtMainWindow.h"

int main(int argc, char** argv) {
    QApplication qt_app(argc, argv);

    cad::app::CADApplication application;
    cad::app::AppController controller;
    application.setController(&controller);
    application.initialize();

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
