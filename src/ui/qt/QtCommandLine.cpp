#include "QtCommandLine.h"

#include <QKeyEvent>
#include <QRegularExpression>

namespace cad {
namespace ui {

QtCommandLine::QtCommandLine(QWidget* parent) : QLineEdit(parent) {
    setPlaceholderText(tr("Command [parameters...]"));
    initializeValidCommands();
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
        QString cmd = takeCurrentCommand();
        if (!cmd.isEmpty()) {
            ParsedCommand parsed = parseCommand(cmd);
            if (validateCommand(parsed)) {
                emit commandParsed(parsed);
            }
        }
    }
    QLineEdit::keyPressEvent(event);
}

ParsedCommand QtCommandLine::parseCommand(const QString& input) const {
    ParsedCommand result;
    
    if (input.trimmed().isEmpty()) {
        result.valid = false;
        result.error_message = tr("Empty command");
        return result;
    }
    
    // Simple parsing: split by spaces, first token is command, rest are parameters
    QStringList parts = input.trimmed().split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    
    if (parts.isEmpty()) {
        result.valid = false;
        result.error_message = tr("No command specified");
        return result;
    }
    
    result.command = parts[0];
    if (parts.size() > 1) {
        result.parameters = parts.mid(1);
    }
    
    result.valid = true;
    return result;
}

bool QtCommandLine::validateCommand(const ParsedCommand& parsed) const {
    if (!parsed.valid) {
        return false;
    }
    
    if (!isValidCommandName(parsed.command)) {
        return false;
    }
    
    if (!validateParameterCount(parsed.command, parsed.parameters.size())) {
        return false;
    }
    
    if (!validateParameterType(parsed.command, parsed.parameters)) {
        return false;
    }
    
    return true;
}

void QtCommandLine::initializeValidCommands() {
    valid_commands_ = {
        "Parameters", "Line", "Rectangle", "Circle", "Arc", "Constraint",
        "Extrude", "Revolve", "Loft", "Hole", "Fillet",
        "Flange", "Bend", "Unfold", "Refold",
        "RectangularPattern", "CircularPattern", "CurvePattern",
        "DirectEdit", "Freeform",
        "LoadAssembly", "Place", "Mate", "Flush", "Angle", "Pattern",
        "RigidPipe", "FlexibleHose", "BentTube", "Simplify",
        "BaseView", "Section", "Dimension", "PartsList",
        "Measure", "Interference", "SectionAnalysis",
        "Simulation", "StressAnalysis",
        "Styles", "AddIns", "Import", "Export", "ExportRFA", "MbdNote",
        "Visibility", "Appearance", "Environment",
        "Illustration", "Rendering", "Animation", "MBDView"
    };
}

bool QtCommandLine::isValidCommandName(const QString& name) const {
    return valid_commands_.contains(name, Qt::CaseInsensitive);
}

bool QtCommandLine::validateParameterCount(const QString& command, int param_count) const {
    // Commands that require parameters
    if (command.compare("Extrude", Qt::CaseInsensitive) == 0 ||
        command.compare("Revolve", Qt::CaseInsensitive) == 0) {
        return param_count >= 1;  // At least depth/angle
    }
    
    if (command.compare("Dimension", Qt::CaseInsensitive) == 0) {
        return param_count >= 0;  // Optional parameters
    }
    
    // Most commands don't require parameters
    return true;
}

bool QtCommandLine::validateParameterType(const QString& command, const QStringList& params) const {
    if (params.isEmpty()) {
        return true;
    }
    
    // Validate numeric parameters for certain commands
    if (command.compare("Extrude", Qt::CaseInsensitive) == 0 ||
        command.compare("Revolve", Qt::CaseInsensitive) == 0 ||
        command.compare("Hole", Qt::CaseInsensitive) == 0) {
        bool ok;
        params[0].toDouble(&ok);
        if (!ok) {
            return false;
        }
    }
    
    return true;
}

}  // namespace ui
}  // namespace cad
