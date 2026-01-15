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

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(fps_label_);
    layout->addWidget(cache_label_);
    layout->addWidget(new QLabel(tr("LOD Mode"), this));
    layout->addWidget(lod_selector_);
    layout->addWidget(background_loading_);
    layout->addStretch();
    setLayout(layout);

    connect(lod_selector_, &QComboBox::currentTextChanged, this, [this](const QString& text) {
        emit lodModeChanged(text);
    });
    connect(background_loading_, &QCheckBox::toggled, this, [this](bool enabled) {
        emit backgroundLoadingToggled(enabled);
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

}  // namespace ui
}  // namespace cad
