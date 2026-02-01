#pragma once

#include <QWidget>

namespace cad {
namespace ui {

/**
 * XYZ-Koordinaten-Widget (Tripod) unten links im Viewport (Inventor-Niveau).
 */
class QtViewportAxesWidget : public QWidget {
    Q_OBJECT

public:
    explicit QtViewportAxesWidget(QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;
};

}  // namespace ui
}  // namespace cad
