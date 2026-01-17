#include "QtPerformancePanel.h"

#include <QVBoxLayout>

namespace cad {
namespace ui {

QtPerformancePanel::QtPerformancePanel(QWidget* parent) : QWidget(parent) {
    fps_label_ = new QLabel(tr("FPS: --"), this);
    cache_label_ = new QLabel(tr("Cache: --/--"), this);
    lod_selector_ = new QComboBox(this);
    lod_selector_->addItems({tr("Full"), tr("Simplified"), tr("Bounding Boxes")});
    lod_selector_->setCurrentIndex(1);
    background_loading_ = new QCheckBox(tr("Background loading"), this);
    background_loading_->setChecked(true);
    progress_label_ = new QLabel(tr("Load progress: --"), this);
    fps_target_label_ = new QLabel(tr("Target FPS: 30"), this);
    fps_slider_ = new QSlider(Qt::Horizontal, this);
    fps_slider_->setRange(5, 120);
    fps_slider_->setValue(30);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(fps_label_);
    layout->addWidget(cache_label_);
    layout->addWidget(progress_label_);
    layout->addWidget(new QLabel(tr("LOD Mode"), this));
    layout->addWidget(lod_selector_);
    layout->addWidget(background_loading_);
    layout->addWidget(fps_target_label_);
    layout->addWidget(fps_slider_);
    layout->addStretch();
    setLayout(layout);

    connect(lod_selector_, &QComboBox::currentTextChanged, this, [this](const QString& text) {
        emit lodModeChanged(text.toLower());
    });
    connect(background_loading_, &QCheckBox::toggled, this, [this](bool enabled) {
        emit backgroundLoadingToggled(enabled);
    });
    connect(fps_slider_, &QSlider::valueChanged, this, [this](int value) {
        fps_target_label_->setText(tr("Target FPS: %1").arg(value));
        emit targetFpsChanged(value);
    });
}

void QtPerformancePanel::setFps(double fps) {
    if (fps_label_) {
        fps_label_->setText(tr("FPS: %1").arg(QString::number(fps, 'f', 0)));
    }
}

void QtPerformancePanel::setCacheStats(int entries, int max_entries) {
    if (cache_label_) {
        cache_label_->setText(tr("Cache: %1/%2").arg(entries).arg(max_entries));
    }
}

void QtPerformancePanel::setBackgroundLoading(bool enabled) {
    if (background_loading_) {
        background_loading_->setChecked(enabled);
    }
}

void QtPerformancePanel::setProgress(int progress) {
    if (progress_label_) {
        if (progress < 0) {
            progress_label_->setText(tr("Load progress: --"));
        } else {
            progress_label_->setText(tr("Load progress: %1%").arg(progress));
        }
    }
}

void QtPerformancePanel::setTargetFps(int fps) {
    if (fps_slider_) {
        fps_slider_->setValue(fps);
    }
    if (fps_target_label_) {
        fps_target_label_->setText(tr("Target FPS: %1").arg(fps));
    }
}

}  // namespace ui
}  // namespace cad
