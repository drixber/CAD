#include "QtViewport.h"

#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace cad {
namespace ui {

QtViewport::QtViewport(QWidget* parent) : QFrame(parent) {
    setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    QVBoxLayout* layout = new QVBoxLayout(this);
    status_label_ = new QLabel(tr("3D Viewport Placeholder"), this);
    status_label_->setAlignment(Qt::AlignCenter);
    layout->addWidget(status_label_);
    nav_label_ = new QLabel(tr("Navigation: orbit"), this);
    nav_label_->setAlignment(Qt::AlignCenter);
    QFont font = nav_label_->font();
    font.setPointSize(font.pointSize() - 1);
    nav_label_->setFont(font);
    layout->addWidget(nav_label_);
    QHBoxLayout* toolbar = new QHBoxLayout();
    QPushButton* orbit = new QPushButton(tr("Orbit"), this);
    QPushButton* pan = new QPushButton(tr("Pan"), this);
    QPushButton* zoom = new QPushButton(tr("Zoom"), this);
    toolbar->addStretch();
    toolbar->addWidget(orbit);
    toolbar->addWidget(pan);
    toolbar->addWidget(zoom);
    toolbar->addStretch();
    layout->addLayout(toolbar);
    connect(orbit, &QPushButton::clicked, this, [this]() { setNavigationMode("orbit"); });
    connect(pan, &QPushButton::clicked, this, [this]() { setNavigationMode("pan"); });
    connect(zoom, &QPushButton::clicked, this, [this]() { setNavigationMode("zoom"); });
    setLayout(layout);
}

void QtViewport::setStatusText(const QString& text) {
    if (status_label_) {
        status_label_->setText(text);
    }
}

void QtViewport::setNavigationMode(const QString& mode) {
    if (nav_label_) {
        nav_label_->setText(tr("Navigation: %1").arg(mode));
    }
    setStatusText(tr("Viewport mode: %1").arg(mode));
}

}  // namespace ui
}  // namespace cad
