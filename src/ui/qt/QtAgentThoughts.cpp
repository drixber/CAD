#include "QtAgentThoughts.h"

#include <QVBoxLayout>

namespace cad {
namespace ui {

QtAgentThoughts::QtAgentThoughts(QWidget* parent) : QWidget(parent) {
    output_ = new QTextEdit(this);
    output_->setReadOnly(true);
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(output_);
    setLayout(layout);
}

void QtAgentThoughts::appendThought(const QString& thought) {
    output_->append(thought);
}

}  // namespace ui
}  // namespace cad
