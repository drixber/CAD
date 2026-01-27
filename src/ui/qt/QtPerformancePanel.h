#pragma once

#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QSlider>
#include <QWidget>

namespace cad {
namespace ui {

class QtPerformancePanel : public ::QWidget {
    Q_OBJECT

public:
    explicit QtPerformancePanel(::QWidget* parent = nullptr);
    void setFps(double fps);
    void setCacheStats(int entries, int max_entries);
    void setBackgroundLoading(bool enabled);
    void setProgress(int progress);
    void setTargetFps(int fps);

signals:
    void lodModeChanged(const QString& mode);
    void backgroundLoadingToggled(bool enabled);
    void targetFpsChanged(int fps);

private:
    QLabel* fps_label_{nullptr};
    QLabel* cache_label_{nullptr};
    QComboBox* lod_selector_{nullptr};
    QCheckBox* background_loading_{nullptr};
    QLabel* progress_label_{nullptr};
    QSlider* fps_slider_{nullptr};
    QLabel* fps_target_label_{nullptr};
};

}  // namespace ui
}  // namespace cad
