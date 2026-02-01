#include "QtViewCubeWidget.h"
#include "viewport/Viewport3D.h"

#include <QGridLayout>
#include <QPushButton>
#include <QFrame>
#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QIcon>

namespace cad {
namespace ui {

QtViewCubeWidget::QtViewCubeWidget(QWidget* parent) : QWidget(parent) {
    setObjectName("viewCubeWidget");
    setStyleSheet(
        "#viewCubeWidget { background-color: rgba(40, 40, 40, 220); border-radius: 4px; }"
        "#viewCubeWidget QPushButton { min-width: 28px; max-height: 22px; font-size: 10px; }"
    );
    setFixedWidth(72);
    QVBoxLayout* v = new QVBoxLayout(this);
    v->setContentsMargins(4, 4, 4, 4);
    v->setSpacing(2);

    // Visual Style (Inventor: Shaded / Wireframe / Hidden Line)
    QLabel* style_label = new QLabel(tr("Style"), this);
    style_label->setStyleSheet("font-size: 9px; color: #aaa;");
    v->addWidget(style_label);
    QHBoxLayout* style_row = new QHBoxLayout();
    btn_shaded_     = new QPushButton(this);
    btn_wireframe_  = new QPushButton(this);
    btn_hidden_line_= new QPushButton(this);
    btn_shaded_->setToolTip(tr("Shaded"));
    btn_wireframe_->setToolTip(tr("Wireframe"));
    btn_hidden_line_->setToolTip(tr("Hidden Line"));
    btn_shaded_->setIcon(QIcon(":/icons/view/shaded.svg"));
    btn_wireframe_->setIcon(QIcon(":/icons/view/wireframe.svg"));
    btn_hidden_line_->setIcon(QIcon(":/icons/view/hidden_line.svg"));
    btn_shaded_->setIconSize(QSize(20, 20));
    btn_wireframe_->setIconSize(QSize(20, 20));
    btn_hidden_line_->setIconSize(QSize(20, 20));
    btn_shaded_->setFixedSize(28, 22);
    btn_wireframe_->setFixedSize(28, 22);
    btn_hidden_line_->setFixedSize(28, 22);
    style_row->addWidget(btn_shaded_);
    style_row->addWidget(btn_wireframe_);
    style_row->addWidget(btn_hidden_line_);
    v->addLayout(style_row);

    connect(btn_shaded_,     &QPushButton::clicked, this, [this]() { onDisplayModeClicked(1); });
    connect(btn_wireframe_,   &QPushButton::clicked, this, [this]() { onDisplayModeClicked(0); });
    connect(btn_hidden_line_, &QPushButton::clicked, this, [this]() { onDisplayModeClicked(2); });

    btn_projection_ = new QPushButton(tr("Persp"), this);
    btn_projection_->setToolTip(tr("Toggle Orthographic / Perspective"));
    btn_projection_->setFixedSize(48, 22);
    connect(btn_projection_, &QPushButton::clicked, this, [this]() {
        if (viewport_) {
            bool ortho = !viewport_->isOrthographic();
            viewport_->setProjectionType(ortho);
            btn_projection_->setText(ortho ? tr("Ortho") : tr("Persp"));
        }
    });
    v->addWidget(btn_projection_);

    btn_top_    = new QPushButton(tr("Top"), this);
    btn_front_  = new QPushButton(tr("Front"), this);
    btn_right_  = new QPushButton(tr("Right"), this);
    btn_left_   = new QPushButton(tr("Left"), this);
    btn_back_   = new QPushButton(tr("Back"), this);
    btn_bottom_ = new QPushButton(tr("Bottom"), this);
    btn_iso_    = new QPushButton(tr("Iso"), this);
    btn_home_   = new QPushButton(tr("Home"), this);

    v->addWidget(btn_top_);
    v->addWidget(btn_front_);
    v->addWidget(btn_right_);
    v->addWidget(btn_left_);
    v->addWidget(btn_back_);
    v->addWidget(btn_bottom_);
    v->addWidget(btn_iso_);
    v->addWidget(btn_home_);
    v->addStretch();

    connect(btn_top_,    &QPushButton::clicked, this, [this]() { onViewClicked("Top");    });
    connect(btn_front_,  &QPushButton::clicked, this, [this]() { onViewClicked("Front");  });
    connect(btn_right_,  &QPushButton::clicked, this, [this]() { onViewClicked("Right");  });
    connect(btn_left_,   &QPushButton::clicked, this, [this]() { onViewClicked("Left");   });
    connect(btn_back_,   &QPushButton::clicked, this, [this]() { onViewClicked("Back");   });
    connect(btn_bottom_, &QPushButton::clicked, this, [this]() { onViewClicked("Bottom"); });
    connect(btn_iso_,    &QPushButton::clicked, this, [this]() { onViewClicked("Isometric"); });
    connect(btn_home_,   &QPushButton::clicked, this, [this]() {
        if (viewport_) {
            viewport_->fitToView();
        }
        emit homeViewRequested();
    });

    // Navigation Bar (Inventor-style: Pan, Zoom, Orbit, Home)
    QLabel* nav_label = new QLabel(tr("Nav"), this);
    nav_label->setStyleSheet("font-size: 9px; color: #aaa;");
    v->addWidget(nav_label);
    QHBoxLayout* nav_row = new QHBoxLayout();
    btn_nav_orbit_ = new QPushButton(this);
    btn_nav_pan_   = new QPushButton(this);
    btn_nav_zoom_  = new QPushButton(this);
    btn_nav_home_  = new QPushButton(this);
    btn_nav_orbit_->setToolTip(tr("Orbit"));
    btn_nav_pan_->setToolTip(tr("Pan"));
    btn_nav_zoom_->setToolTip(tr("Zoom"));
    btn_nav_home_->setToolTip(tr("Home / Fit"));
    btn_nav_home_->setIcon(QIcon(":/icons/general/zoom_fit.svg"));
    btn_nav_home_->setIconSize(QSize(20, 20));
    btn_nav_orbit_->setText(tr("O"));
    btn_nav_pan_->setText(tr("P"));
    btn_nav_zoom_->setText(tr("Z"));
    for (QPushButton* b : {btn_nav_orbit_, btn_nav_pan_, btn_nav_zoom_, btn_nav_home_}) {
        b->setFixedSize(28, 22);
    }
    nav_row->addWidget(btn_nav_orbit_);
    nav_row->addWidget(btn_nav_pan_);
    nav_row->addWidget(btn_nav_zoom_);
    nav_row->addWidget(btn_nav_home_);
    v->addLayout(nav_row);

    connect(btn_nav_orbit_, &QPushButton::clicked, this, [this]() {
        if (viewport_) {
            viewport_->setPreferredDragMode("orbit");
            emit navigationModeChanged(tr("Orbit"));
        }
    });
    connect(btn_nav_pan_,   &QPushButton::clicked, this, [this]() {
        if (viewport_) {
            viewport_->setPreferredDragMode("pan");
            emit navigationModeChanged(tr("Pan"));
        }
    });
    connect(btn_nav_zoom_,  &QPushButton::clicked, this, [this]() {
        if (viewport_) {
            viewport_->setPreferredDragMode("zoom");
            emit navigationModeChanged(tr("Zoom"));
        }
    });
    connect(btn_nav_home_,  &QPushButton::clicked, this, [this]() {
        if (viewport_) viewport_->fitToView();
        emit homeViewRequested();
    });
}

void QtViewCubeWidget::onViewClicked(const QString& view) {
    if (viewport_) {
        viewport_->setStandardView(view.toStdString());
    }
    emit standardViewRequested(view);
}

void QtViewCubeWidget::onDisplayModeClicked(int mode) {
    if (viewport_) {
        viewport_->setDisplayMode(static_cast<Viewport3D::DisplayMode>(mode));
    }
    emit displayModeChanged(mode);
}

}  // namespace ui
}  // namespace cad
