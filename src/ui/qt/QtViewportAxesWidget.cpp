#include "QtViewportAxesWidget.h"

#include <QPainter>
#include <QPen>

namespace cad {
namespace ui {

QtViewportAxesWidget::QtViewportAxesWidget(QWidget* parent) : QWidget(parent) {
    setFixedSize(56, 56);
    setObjectName("viewportAxesWidget");
    setStyleSheet("#viewportAxesWidget { background-color: rgba(40, 40, 40, 200); border-radius: 4px; }");
    setAttribute(Qt::WA_TransparentForMouseEvents, false);
}

void QtViewportAxesWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    const int cx = width() / 2;
    const int cy = height() / 2;
    const int len = 20;

    // X (red) – right
    p.setPen(QPen(QColor(200, 80, 80), 2));
    p.drawLine(cx, cy, cx + len, cy);
    p.drawText(cx + len + 2, cy + 4, "X");

    // Y (green) – up
    p.setPen(QPen(QColor(80, 200, 80), 2));
    p.drawLine(cx, cy, cx, cy - len);
    p.drawText(cx + 2, cy - len - 4, "Y");

    // Z (blue) – diagonal (out of screen)
    p.setPen(QPen(QColor(80, 80, 200), 2));
    p.drawLine(cx, cy, cx - len/2, cy + len/2);
    p.drawText(cx - len/2 - 14, cy + len/2 + 4, "Z");
}

}  // namespace ui
}  // namespace cad
