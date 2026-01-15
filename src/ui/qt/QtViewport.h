#pragma once

#include <QFrame>
#include <QLabel>
#include <QTimer>

namespace cad {
namespace ui {

class QtViewport : public QFrame {
    Q_OBJECT

public:
    explicit QtViewport(QWidget* parent = nullptr);
    void setStatusText(const QString& text);
    void setNavigationMode(const QString& mode);
    void setFps(double fps);

signals:
    void fpsUpdated(double fps);

private:
    QLabel* status_label_{nullptr};
    QLabel* nav_label_{nullptr};
    QLabel* fps_label_{nullptr};
    QTimer* fps_timer_{nullptr};
};

}  // namespace ui
}  // namespace cad
