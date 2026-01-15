#include "QtLogPanel.h"

#include <QVBoxLayout>

namespace cad {
namespace ui {

QtLogPanel::QtLogPanel(QWidget* parent) : QWidget(parent) {
    output_ = new QTextEdit(this);
    output_->setReadOnly(true);
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(output_);
    setLayout(layout);
}

void QtLogPanel::appendLog(const QString& message) {
    output_->append(message);
}

}  // namespace ui
}  // namespace cad
