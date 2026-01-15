#pragma once

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

private:
    QLabel* fps_label_{nullptr};
    QLabel* cache_label_{nullptr};
};

}  // namespace ui
}  // namespace cad
