#pragma once

#include <QWidget>

#include <QLabel>

namespace cad {
namespace ui {

class QtPropertyPanel : public QWidget {
    Q_OBJECT

public:
    explicit QtPropertyPanel(QWidget* parent = nullptr);
    void setConstraintCount(int count);
    void setParameterCount(int count);
    void setParameterSummary(const QString& summary);
    void setIntegrationStatus(const QString& status);
    void setMateCount(int count);

private:
    QLabel* constraints_label_{nullptr};
    QLabel* parameters_label_{nullptr};
    QLabel* parameters_summary_{nullptr};
    QLabel* integration_status_{nullptr};
    QLabel* mates_label_{nullptr};
};

}  // namespace ui
}  // namespace cad
