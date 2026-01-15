#include "QtCommandLine.h"

#include <QKeyEvent>

namespace cad {
namespace ui {

QtCommandLine::QtCommandLine(QWidget* parent) : QLineEdit(parent) {
    setPlaceholderText(tr("Command"));
}

void QtCommandLine::setHistory(const QStringList& history) {
    history_ = history;
    history_index_ = history_.isEmpty() ? -1 : history_.size();
}

const QStringList& QtCommandLine::history() const {
    return history_;
}

QString QtCommandLine::takeCurrentCommand() {
    const QString trimmed = text().trimmed();
    if (!trimmed.isEmpty()) {
        history_.append(trimmed);
        history_index_ = history_.size();
        clear();
    }
    return trimmed;
}

void QtCommandLine::keyPressEvent(QKeyEvent* event) {
    if (history_.isEmpty()) {
        QLineEdit::keyPressEvent(event);
        return;
    }
    if (event->key() == Qt::Key_Up) {
        if (history_index_ > 0) {
            history_index_--;
            setText(history_.at(history_index_));
        }
        return;
    }
    if (event->key() == Qt::Key_Down) {
        if (history_index_ < history_.size() - 1) {
            history_index_++;
            setText(history_.at(history_index_));
        } else {
            history_index_ = history_.size();
            clear();
        }
        return;
    }
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        takeCurrentCommand();
    }
    QLineEdit::keyPressEvent(event);
}

}  // namespace ui
}  // namespace cad
