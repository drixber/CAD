#include "QtViewport.h"
#include "QtViewCubeWidget.h"
#include "QtViewportAxesWidget.h"

#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QResizeEvent>
#include <QVBoxLayout>
#include "viewport/Viewport3D.h"

namespace cad {
namespace ui {

QtViewport::QtViewport(QWidget* parent) : QFrame(parent) {
    setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    setObjectName("cadViewport");
    setStyleSheet("#cadViewport { background-color: #2b2b2b; }");
    layout_ = new QVBoxLayout(this);
    layout_->setContentsMargins(0, 0, 0, 0);
    
    // Wrapper for viewport + ViewCube overlay (Inventor-style)
    viewport_container_ = new QWidget(this);
    QVBoxLayout* container_layout = new QVBoxLayout(viewport_container_);
    container_layout->setContentsMargins(0, 0, 0, 0);
    
    viewport_3d_ = new Viewport3D(viewport_container_);
    viewport_3d_->setMinimumSize(400, 320);
    viewport_3d_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    container_layout->addWidget(viewport_3d_);
    
    viewcube_widget_ = new QtViewCubeWidget(viewport_container_);
    viewcube_widget_->setViewport(viewport_3d_);
    viewcube_widget_->setParent(viewport_container_);
    viewcube_widget_->raise();

    axes_widget_ = new QtViewportAxesWidget(viewport_container_);
    axes_widget_->setParent(viewport_container_);
    axes_widget_->raise();
    
    layout_->addWidget(viewport_container_);
    
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
    QPushButton* zoom_sel = new QPushButton(tr("Zoom Selection"), this);
    zoom_sel->setToolTip(tr("Zoom to selection (or fit all if nothing selected)"));
    toolbar->addStretch();
    toolbar->addWidget(orbit);
    toolbar->addWidget(pan);
    toolbar->addWidget(zoom);
    toolbar->addWidget(fit);
    toolbar->addWidget(zoom_sel);
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
    connect(zoom_sel, &QPushButton::clicked, this, [this]() {
        if (viewport_3d_) {
            viewport_3d_->fitToSelection();
        }
    });
    
    connect(viewport_3d_, &Viewport3D::viewportUpdated, this, [this]() {
        setFps(60.0);
    });
    if (viewcube_widget_) {
        connect(viewcube_widget_, &QtViewCubeWidget::navigationModeChanged, this, [this](const QString& mode) {
            setNavigationMode(mode);
        });
    }

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

void QtViewport::resizeEvent(QResizeEvent* event) {
    QFrame::resizeEvent(event);
    const int margin = 8;
    if (viewport_container_) {
        if (viewcube_widget_) {
            viewcube_widget_->adjustSize();
            viewcube_widget_->move(
                viewport_container_->width() - viewcube_widget_->width() - margin,
                margin
            );
        }
        if (axes_widget_) {
            axes_widget_->move(margin, viewport_container_->height() - axes_widget_->height() - margin);
        }
    }
}

}  // namespace ui
}  // namespace cad
