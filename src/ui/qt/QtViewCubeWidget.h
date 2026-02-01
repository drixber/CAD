#pragma once

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>

namespace cad {
namespace ui {

class Viewport3D;

/**
 * ViewCube-style overlay: Standardansichten (Top, Front, Right, …) und Home.
 * Wird oben rechts im Viewport eingeblendet (Inventor-Niveau).
 */
class QtViewCubeWidget : public QWidget {
    Q_OBJECT

public:
    explicit QtViewCubeWidget(QWidget* parent = nullptr);

    void setViewport(Viewport3D* viewport) { viewport_ = viewport; }
    Viewport3D* viewport() const { return viewport_; }

signals:
    void standardViewRequested(const QString& view);
    void homeViewRequested();
    void displayModeChanged(int mode);
    void navigationModeChanged(const QString& mode);

private:
    void onViewClicked(const QString& view);
    void onDisplayModeClicked(int mode);

    Viewport3D* viewport_{nullptr};
    QPushButton* btn_shaded_{nullptr};
    QPushButton* btn_wireframe_{nullptr};
    QPushButton* btn_hidden_line_{nullptr};
    QPushButton* btn_projection_{nullptr};  // Ortho / Persp toggle
    QPushButton* btn_top_{nullptr};
    QPushButton* btn_front_{nullptr};
    QPushButton* btn_right_{nullptr};
    QPushButton* btn_left_{nullptr};
    QPushButton* btn_back_{nullptr};
    QPushButton* btn_bottom_{nullptr};
    QPushButton* btn_iso_{nullptr};
    QPushButton* btn_home_{nullptr};
    // Navigation Bar (unter ViewCube): Orbit, Pan, Zoom, Home
    QPushButton* btn_nav_orbit_{nullptr};
    QPushButton* btn_nav_pan_{nullptr};
    QPushButton* btn_nav_zoom_{nullptr};
    QPushButton* btn_nav_home_{nullptr};
};

}  // namespace ui
}  // namespace cad
