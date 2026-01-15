#pragma once

#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QWidget>

namespace cad {
namespace ui {

class QtPerformancePanel : public QWidget {
    Q_OBJECT

public:
    explicit QtPerformancePanel(QWidget* parent = nullptr);
    void setFps(double fps);
    void setCacheStats(int entries, int max_entries);
    void setBackgroundLoading(bool enabled);

signals:
    void lodModeChanged(const QString& mode);
    void backgroundLoadingToggled(bool enabled);

private:
    QLabel* fps_label_{nullptr};
    QLabel* cache_label_{nullptr};
    QComboBox* lod_selector_{nullptr};
    QCheckBox* background_loading_{nullptr};
};

}  // namespace ui
}  // namespace cad
