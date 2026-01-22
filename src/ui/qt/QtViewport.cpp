#include "QtViewport.h"

#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include "viewport/Viewport3D.h"

namespace cad {
namespace ui {

QtViewport::QtViewport(QWidget* parent) : QFrame(parent) {
    setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    layout_ = new QVBoxLayout(this);
    layout_->setContentsMargins(0, 0, 0, 0);
    
    // Create 3D viewport
    viewport_3d_ = new Viewport3D(this);
    layout_->addWidget(viewport_3d_);
    
    // Status bar at bottom
    QHBoxLayout* status_layout = new QHBoxLayout();
    status_label_ = new QLabel(tr("3D Viewport"), this);
    status_label_->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    status_layout->addWidget(status_label_);
    
    nav_label_ = new QLabel(tr("Navigation: orbit"), this);
    nav_label_->setAlignment(Qt::AlignCenter);
    QFont font = nav_label_->font();
    font.setPointSize(font.pointSize() - 1);
    nav_label_->setFont(font);
    status_layout->addWidget(nav_label_);
    
    fps_label_ = new QLabel(tr("FPS: --"), this);
    fps_label_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    QFont fps_font = fps_label_->font();
    fps_font.setPointSize(fps_font.pointSize() - 1);
    fps_label_->setFont(fps_font);
    status_layout->addWidget(fps_label_);
    
    layout_->addLayout(status_layout);
    
    // Navigation toolbar
    QHBoxLayout* toolbar = new QHBoxLayout();
    QPushButton* orbit = new QPushButton(tr("Orbit"), this);
    QPushButton* pan = new QPushButton(tr("Pan"), this);
    QPushButton* zoom = new QPushButton(tr("Zoom"), this);
    QPushButton* fit = new QPushButton(tr("Fit"), this);
    toolbar->addStretch();
    toolbar->addWidget(orbit);
    toolbar->addWidget(pan);
    toolbar->addWidget(zoom);
    toolbar->addWidget(fit);
    toolbar->addStretch();
    layout_->addLayout(toolbar);
    
    connect(orbit, &QPushButton::clicked, this, [this]() { 
        setNavigationMode("orbit"); 
        if (viewport_3d_) {
            viewport_3d_->resetCamera();
        }
    });
    connect(pan, &QPushButton::clicked, this, [this]() { setNavigationMode("pan"); });
    connect(zoom, &QPushButton::clicked, this, [this]() { setNavigationMode("zoom"); });
    connect(fit, &QPushButton::clicked, this, [this]() { 
        if (viewport_3d_) {
            viewport_3d_->fitToView();
        }
    });
    
    connect(viewport_3d_, &Viewport3D::viewportUpdated, this, [this]() {
        // Update FPS when viewport updates
        setFps(60.0);
    });

    fps_timer_ = new QTimer(this);
    connect(fps_timer_, &QTimer::timeout, this, [this]() { setFps(60.0); });
    fps_timer_->start(1000);
    setLayout(layout_);
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

void QtViewport::setFps(double fps) {
    if (fps_label_) {
        fps_label_->setText(tr("FPS: %1").arg(QString::number(fps, 'f', 0)));
    }
    emit fpsUpdated(fps);
}

}  // namespace ui
}  // namespace cad
