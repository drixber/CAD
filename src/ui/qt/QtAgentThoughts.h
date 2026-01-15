#pragma once

#include <QTextEdit>
#include <QWidget>

namespace cad {
namespace ui {

class QtAgentThoughts : public QWidget {
    Q_OBJECT

public:
    explicit QtAgentThoughts(QWidget* parent = nullptr);
    void appendThought(const QString& thought);

private:
    QTextEdit* output_{nullptr};
};

}  // namespace ui
}  // namespace cad
