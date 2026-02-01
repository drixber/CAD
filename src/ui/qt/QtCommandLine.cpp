#include "QtCommandLine.h"

#include <QCompleter>
#include <QKeyEvent>
#include <QRegularExpression>
#include <QToolTip>
#include <QPoint>
#include <QStringListModel>

namespace cad {
namespace ui {

QtCommandLine::QtCommandLine(QWidget* parent) : QLineEdit(parent) {
    setPlaceholderText(tr("Command [parameters...]"));
    initializeValidCommands();
    initializeCommandDefinitions();
    
    // Setup completer
    completion_model_ = new QStringListModel(this);
    completer_ = new QCompleter(completion_model_, this);
    completer_->setCaseSensitivity(Qt::CaseInsensitive);
    completer_->setCompletionMode(QCompleter::PopupCompletion);
    setCompleter(completer_);
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
    if (event->key() == Qt::Key_Tab) {
        // Handle tab completion
        QString current_text = text();
        QStringList completions = getCommandCompletions(current_text);
        if (!completions.isEmpty()) {
            emit commandCompletionRequested(current_text, completions);
            if (completions.size() == 1) {
                // Auto-complete if single match
                setText(completions[0] + " ");
            } else {
                // Update completer model
                completion_model_->setStringList(completions);
                completer_->complete();
            }
        }
        event->accept();
        return;
    }
    if (event->key() == Qt::Key_F1) {
        // Show help for current command
        QString current_text = text().trimmed();
        QStringList parts = current_text.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
        if (!parts.isEmpty()) {
            QString cmd = parts[0];
            if (isValidCommandName(cmd)) {
                showCommandTooltip(cmd);
                emit helpRequested(cmd);
            }
        }
        event->accept();
        return;
    }
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        QString cmd = takeCurrentCommand();
        if (!cmd.isEmpty()) {
            ParsedCommand parsed = parseCommand(cmd);
            if (validateCommand(parsed)) {
                emit commandParsed(parsed);
            } else if (!parsed.error_message.isEmpty()) {
                emit validationError(parsed.error_message);
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

bool QtCommandLine::validateCommand(ParsedCommand& parsed) const {
    if (!parsed.valid) {
        return false;
    }
    
    if (!isValidCommandName(parsed.command)) {
        parsed.valid = false;
        parsed.error_message = tr("Unknown command: %1").arg(parsed.command);
        parsed.suggestions = getCommandSuggestions(parsed.command);
        return false;
    }
    
    if (!validateParameterCount(parsed.command, parsed.parameters.size())) {
        parsed.valid = false;
        QString cmd_lower = parsed.command.toLower();
        if (command_definitions_.contains(cmd_lower)) {
            const CommandDefinition& def = command_definitions_[cmd_lower];
            parsed.error_message = tr("Invalid parameter count for %1. Expected %2-%3, got %4")
                .arg(parsed.command)
                .arg(def.min_parameters)
                .arg(def.max_parameters > 0 ? QString::number(def.max_parameters) : tr("unlimited"))
                .arg(parsed.parameters.size());
        } else {
            parsed.error_message = tr("Invalid parameter count for %1").arg(parsed.command);
        }
        return false;
    }
    
    if (!validateParameterType(parsed.command, parsed.parameters, parsed)) {
        return false;
    }
    
    return true;
}

void QtCommandLine::initializeValidCommands() {
    valid_commands_ = QStringList({
        "New", "Open", "Save", "Import", "Export", "GetStarted", "Documentation",
        "Parameters", "Line", "Rectangle", "Circle", "Arc", "Constraint",
        "Extrude", "Revolve", "Loft", "Hole", "HoleThroughAll", "Fillet", "Chamfer", "Shell", "Mirror",
        "Flange", "Bend", "Unfold", "Refold", "Punch", "Bead", "SheetMetalRules", "ExportFlatDXF",
        "RectangularPattern", "CircularPattern", "CurvePattern", "FacePattern",
        "DirectEdit", "Freeform",
        "LoadAssembly", "Place", "Mate", "Flush", "Angle", "Parallel", "Distance", "Pattern", "ExplosionView",
        "RigidPipe", "FlexibleHose", "BentTube", "RouteBOM", "Weld", "WeldBOM", "Simplify",
        "BaseView", "Section", "DetailView", "Dimension", "PartsList",
        "Measure", "Interference", "SectionAnalysis",
        "Simulation", "StressAnalysis", "ExportFEAReport", "ExportMotionReport",
        "Styles", "AddIns", "Import", "Export", "ExportRFA", "MbdNote",
        "Visibility", "Appearance", "Environment",
        "Illustration", "Rendering", "Animation", "MBDView", "MbdView",
        "Shaded", "Wireframe", "HiddenLine", "Undo", "Redo"
    });
    
    // Update completer model
    if (completion_model_) {
        completion_model_->setStringList(valid_commands_);
    }
}

void QtCommandLine::initializeCommandDefinitions() {
    // Extrude command
    CommandDefinition extrude;
    extrude.name = "Extrude";
    extrude.description = tr("Extrude a sketch profile");
    extrude.min_parameters = 1;
    extrude.max_parameters = 3;
    CommandParameter depth;
    depth.name = "depth";
    depth.type = "double";
    depth.description = tr("Extrusion depth");
    depth.required = true;
    depth.has_range = true;
    depth.min_value = 0.1;
    depth.max_value = 10000.0;
    extrude.parameters.append(depth);
    command_definitions_["extrude"] = extrude;
    
    // Revolve command
    CommandDefinition revolve;
    revolve.name = "Revolve";
    revolve.description = tr("Revolve a sketch profile");
    revolve.min_parameters = 1;
    revolve.max_parameters = 2;
    CommandParameter angle;
    angle.name = "angle";
    angle.type = "double";
    angle.description = tr("Revolution angle in degrees");
    angle.required = true;
    angle.has_range = true;
    angle.min_value = 0.0;
    angle.max_value = 360.0;
    revolve.parameters.append(angle);
    command_definitions_["revolve"] = revolve;
    
    // Hole command
    CommandDefinition hole;
    hole.name = "Hole";
    hole.description = tr("Create a hole");
    hole.min_parameters = 1;
    hole.max_parameters = 2;
    CommandParameter diameter;
    diameter.name = "diameter";
    diameter.type = "double";
    diameter.description = tr("Hole diameter");
    diameter.required = true;
    diameter.has_range = true;
    diameter.min_value = 0.1;
    diameter.max_value = 1000.0;
    hole.parameters.append(diameter);
    CommandParameter depth_param;
    depth_param.name = "depth";
    depth_param.type = "double";
    depth_param.description = tr("Hole depth (optional)");
    depth_param.required = false;
    hole.parameters.append(depth_param);
    command_definitions_["hole"] = hole;
    
    // Dimension command
    CommandDefinition dimension;
    dimension.name = "Dimension";
    dimension.description = tr("Add a dimension");
    dimension.min_parameters = 0;
    dimension.max_parameters = 0;
    command_definitions_["dimension"] = dimension;
    
    // Mate command: type (required) + optional value (for Distance, Angle, Parallel offset)
    CommandDefinition mate;
    mate.name = "Mate";
    mate.description = tr("Create a mate constraint (type and optional value, e.g. Mate Distance 25)");
    mate.min_parameters = 1;
    mate.max_parameters = 2;
    CommandParameter mate_type;
    mate_type.name = "type";
    mate_type.type = "enum";
    mate_type.description = tr("Mate type");
    mate_type.required = true;
    mate_type.enum_values = QStringList({"Coincident", "Parallel", "Perpendicular", "Tangent", "Concentric", "Distance"});
    mate.parameters.append(mate_type);
    CommandParameter mate_value;
    mate_value.name = "value";
    mate_value.type = "double";
    mate_value.description = tr("Distance (mm) or angle (deg), optional");
    mate_value.required = false;
    mate.parameters.append(mate_value);
    command_definitions_["mate"] = mate;
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

bool QtCommandLine::validateParameterType(const QString& command, const QStringList& params, ParsedCommand& parsed) const {
    if (params.isEmpty()) {
        return true;
    }
    
    QString cmd_lower = command.toLower();
    if (!command_definitions_.contains(cmd_lower)) {
        return true;  // No definition, skip advanced validation
    }
    
    const CommandDefinition& def = command_definitions_[cmd_lower];
    for (int i = 0; i < params.size() && i < def.parameters.size(); ++i) {
        const CommandParameter& param = def.parameters[i];
        const QString& value = params[i];
        
        // Type validation
        if (param.type == "int" || param.type == "double") {
            bool ok;
            double num_value = value.toDouble(&ok);
            if (!ok) {
                parsed.valid = false;
                parsed.error_message = tr("Parameter %1 (%2) must be a number, got: %3")
                    .arg(i + 1)
                    .arg(param.name)
                    .arg(value);
                return false;
            }
            
            // Range validation
            if (param.has_range) {
                if (!validateParameterRange(param, value, parsed)) {
                    return false;
                }
            }
        } else if (param.type == "enum") {
            if (!param.enum_values.isEmpty() && !param.enum_values.contains(value, Qt::CaseInsensitive)) {
                parsed.valid = false;
                parsed.error_message = tr("Parameter %1 (%2) must be one of: %3, got: %4")
                    .arg(i + 1)
                    .arg(param.name)
                    .arg(param.enum_values.join(", "))
                    .arg(value);
                return false;
            }
        }
    }
    
    return true;
}

bool QtCommandLine::validateParameterRange(const CommandParameter& param, const QString& value, ParsedCommand& parsed) const {
    bool ok;
    double num_value = value.toDouble(&ok);
    if (!ok) {
        return false;
    }
    
    if (param.has_range) {
        if (num_value < param.min_value || num_value > param.max_value) {
            parsed.valid = false;
            parsed.error_message = tr("Parameter %1 must be between %2 and %3, got: %4")
                .arg(param.name)
                .arg(param.min_value)
                .arg(param.max_value)
                .arg(value);
            return false;
        }
    }
    
    return true;
}

QStringList QtCommandLine::getCommandCompletions(const QString& prefix) const {
    QString prefix_lower = prefix.trimmed().toLower();
    if (prefix_lower.isEmpty()) {
        return valid_commands_;
    }
    
    QStringList completions;
    for (const QString& cmd : valid_commands_) {
        if (cmd.toLower().startsWith(prefix_lower)) {
            completions.append(cmd);
        }
    }
    
    return completions;
}

QStringList QtCommandLine::getParameterCompletions(const QString& command, int param_index) const {
    QString cmd_lower = command.toLower();
    if (!command_definitions_.contains(cmd_lower)) {
        return QStringList();
    }
    
    const CommandDefinition& def = command_definitions_[cmd_lower];
    if (param_index < 0 || param_index >= def.parameters.size()) {
        return QStringList();
    }
    
    const CommandParameter& param = def.parameters[param_index];
    if (param.type == "enum") {
        return param.enum_values;
    }
    
    return QStringList();
}

QString QtCommandLine::getCommandHelp(const QString& command) const {
    QString cmd_lower = command.toLower();
    if (!command_definitions_.contains(cmd_lower)) {
        return tr("No help available for command: %1").arg(command);
    }
    
    const CommandDefinition& def = command_definitions_[cmd_lower];
    QString help = def.description + "\n\n";
    help += tr("Parameters:\n");
    
    for (int i = 0; i < def.parameters.size(); ++i) {
        const CommandParameter& param = def.parameters[i];
        help += QString("  %1. %2 (%3)").arg(i + 1).arg(param.name).arg(param.type);
        if (param.required) {
            help += " [required]";
        }
        if (!param.description.isEmpty()) {
            help += ": " + param.description;
        }
        if (param.type == "enum" && !param.enum_values.isEmpty()) {
            help += " (" + param.enum_values.join(", ") + ")";
        }
        if (param.has_range) {
            help += QString(" [range: %1-%2]").arg(param.min_value).arg(param.max_value);
        }
        help += "\n";
    }
    
    return help;
}

QStringList QtCommandLine::getCommandSuggestions(const QString& partial) const {
    QString partial_lower = partial.toLower();
    QStringList suggestions;
    
    for (const QString& cmd : valid_commands_) {
        if (cmd.toLower().contains(partial_lower) || partial_lower.contains(cmd.toLower())) {
            suggestions.append(cmd);
        }
    }
    
    return suggestions;
}

QString QtCommandLine::getParameterHint(const QString& command, int param_index) const {
    QString cmd_lower = command.toLower();
    if (!command_definitions_.contains(cmd_lower)) {
        return QString();
    }
    
    const CommandDefinition& def = command_definitions_[cmd_lower];
    if (param_index < 0 || param_index >= def.parameters.size()) {
        return QString();
    }
    
    const CommandParameter& param = def.parameters[param_index];
    QString hint = param.name + " (" + param.type + ")";
    if (!param.description.isEmpty()) {
        hint += ": " + param.description;
    }
    if (param.type == "enum" && !param.enum_values.isEmpty()) {
        hint += " [" + param.enum_values.join(", ") + "]";
    }
    if (param.has_range) {
        hint += QString(" [%1-%2]").arg(param.min_value).arg(param.max_value);
    }
    return hint;
}

void QtCommandLine::showCommandTooltip(const QString& command) {
    QString help = getCommandHelp(command);
    setToolTip(help);
    
    QToolTip::showText(mapToGlobal(QPoint(0, height())), help, this, 
        QRect(0, 0, width(), height()), 5000);
}

void QtCommandLine::showParameterHint(const QString& command, int param_index) {
    QString hint = getParameterHint(command, param_index);
    if (!hint.isEmpty()) {
        setToolTip(hint);
        emit parameterHintRequested(command, param_index);
    }
}

}  // namespace ui
}  // namespace cad
