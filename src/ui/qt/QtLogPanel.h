#pragma once

#include <QTextEdit>
#include <QWidget>

namespace cad {
namespace ui {

class QtLogPanel : public QWidget {
    Q_OBJECT

public:
    explicit QtLogPanel(QWidget* parent = nullptr);
    void appendLog(const QString& message);

private:
    QTextEdit* output_{nullptr};
};

}  // namespace ui
}  // namespace cad
