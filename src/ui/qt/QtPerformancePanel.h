#pragma once

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

signals:
    void lodModeChanged(const QString& mode);

private:
    QLabel* fps_label_{nullptr};
    QLabel* cache_label_{nullptr};
    QComboBox* lod_selector_{nullptr};
};

}  // namespace ui
}  // namespace cad
