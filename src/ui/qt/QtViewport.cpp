#include "QtViewport.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QFont>

namespace cad {
namespace ui {

QtViewport::QtViewport(QWidget* parent) : QFrame(parent) {
    setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    QVBoxLayout* layout = new QVBoxLayout(this);
    status_label_ = new QLabel(tr("3D Viewport Placeholder"), this);
    status_label_->setAlignment(Qt::AlignCenter);
    layout->addWidget(status_label_);
    QLabel* nav_hint = new QLabel(tr("Navigation: orbit/pan/zoom"), this);
    nav_hint->setAlignment(Qt::AlignCenter);
    QFont font = nav_hint->font();
    font.setPointSize(font.pointSize() - 1);
    nav_hint->setFont(font);
    layout->addWidget(nav_hint);
    setLayout(layout);
}

void QtViewport::setStatusText(const QString& text) {
    if (status_label_) {
        status_label_->setText(text);
    }
}

}  // namespace ui
}  // namespace cad
