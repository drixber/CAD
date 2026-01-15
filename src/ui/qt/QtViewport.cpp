#include "QtViewport.h"

#include <QLabel>
#include <QVBoxLayout>

namespace cad {
namespace ui {

QtViewport::QtViewport(QWidget* parent) : QFrame(parent) {
    setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    QVBoxLayout* layout = new QVBoxLayout(this);
    QLabel* label = new QLabel(tr("3D Viewport Placeholder"), this);
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);
    setLayout(layout);
}

}  // namespace ui
}  // namespace cad
