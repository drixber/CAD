#pragma once

#include <QFrame>
#include <QLabel>

namespace cad {
namespace ui {

class QtViewport : public QFrame {
    Q_OBJECT

public:
    explicit QtViewport(QWidget* parent = nullptr);
    void setStatusText(const QString& text);
    void setNavigationMode(const QString& mode);

private:
    QLabel* status_label_{nullptr};
    QLabel* nav_label_{nullptr};
};

}  // namespace ui
}  // namespace cad
