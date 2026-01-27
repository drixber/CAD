#include "QtAgentConsole.h"

#include <QHBoxLayout>
#include <QVBoxLayout>

namespace cad {
namespace ui {

QtAgentConsole::QtAgentConsole(::QWidget* parent) : ::QWidget(parent) {
    output_ = new QTextEdit(this);
    output_->setReadOnly(true);

    input_ = new QLineEdit(this);
    send_ = new QPushButton(tr("Send"), this);

    QHBoxLayout* inputRow = new QHBoxLayout();
    inputRow->addWidget(input_);
    inputRow->addWidget(send_);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(output_);
    layout->addLayout(inputRow);
    setLayout(layout);

    connect(send_, &QPushButton::clicked, this, [this]() {
        const QString command = input_->text().trimmed();
        if (!command.isEmpty()) {
            appendMessage(QString("User: %1").arg(command));
            emit commandIssued(command);
            input_->clear();
        }
    });
}

void QtAgentConsole::appendMessage(const QString& message) {
    output_->append(message);
}

}  // namespace ui
}  // namespace cad
