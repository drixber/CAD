#pragma once

#include <QFrame>
#include <QLabel>
#include <QTimer>
#include <QVBoxLayout>

#include "viewport/Viewport3D.h"

namespace cad {
namespace ui {

class QtViewCubeWidget;
class QtViewportAxesWidget;

class QtViewport : public QFrame {
    Q_OBJECT

public:
    explicit QtViewport(QWidget* parent = nullptr);
    void setStatusText(const QString& text);
    void setNavigationMode(const QString& mode);
    void setFps(double fps);
    
    Viewport3D* viewport3D() { return viewport_3d_; }

signals:
    void fpsUpdated(double fps);

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    Viewport3D* viewport_3d_{nullptr};
    QtViewCubeWidget* viewcube_widget_{nullptr};
    QtViewportAxesWidget* axes_widget_{nullptr};
    QWidget* viewport_container_{nullptr};
    QLabel* status_label_{nullptr};
    QLabel* nav_label_{nullptr};
    QLabel* fps_label_{nullptr};
    QTimer* fps_timer_{nullptr};
    QVBoxLayout* layout_{nullptr};
};

}  // namespace ui
}  // namespace cad
