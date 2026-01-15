#pragma once

#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QWidget>

namespace cad {
namespace ui {

class QtAgentConsole : public QWidget {
    Q_OBJECT

public:
    explicit QtAgentConsole(QWidget* parent = nullptr);
    void appendMessage(const QString& message);

signals:
    void commandIssued(const QString& command);

private:
    QTextEdit* output_{nullptr};
    QLineEdit* input_{nullptr};
    QPushButton* send_{nullptr};
};

}  // namespace ui
}  // namespace cad
