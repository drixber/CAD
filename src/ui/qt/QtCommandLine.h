#pragma once

#include <QCompleter>
#include <QLineEdit>
#include <QStringList>
#include <QStringListModel>

namespace cad {
namespace ui {

struct CommandParameter {
    QString name;
    QString type;  // "int", "double", "string", "enum", "file"
    QString description;
    bool required{false};
    QStringList enum_values;
    double min_value{0.0};
    double max_value{0.0};
    bool has_range{false};
};

struct CommandDefinition {
    QString name;
    QString description;
    QList<CommandParameter> parameters;
    int min_parameters{0};
    int max_parameters{0};
};

struct ParsedCommand {
    QString command;
    QStringList parameters;
    bool valid{true};
    QString error_message;
    QStringList suggestions;
};

class QtCommandLine : public QLineEdit {
    Q_OBJECT

public:
    explicit QtCommandLine(QWidget* parent = nullptr);
    void setHistory(const QStringList& history);
    const QStringList& history() const;
    QString takeCurrentCommand();
    ParsedCommand parseCommand(const QString& input) const;
    bool validateCommand(ParsedCommand& parsed) const;
    QStringList getCommandCompletions(const QString& prefix) const;
    QStringList getParameterCompletions(const QString& command, int param_index) const;
    QString getCommandHelp(const QString& command) const;

signals:
    void commandParsed(const ParsedCommand& parsed);
    void commandCompletionRequested(const QString& prefix, const QStringList& completions);
    void validationError(const QString& error);

protected:
    void keyPressEvent(QKeyEvent* event) override;

private:
    QStringList history_{};
    int history_index_{-1};
    QStringList valid_commands_;
    QMap<QString, CommandDefinition> command_definitions_;
    QCompleter* completer_{nullptr};
    QStringListModel* completion_model_{nullptr};
    
    void initializeValidCommands();
    void initializeCommandDefinitions();
    bool isValidCommandName(const QString& name) const;
    bool validateParameterCount(const QString& command, int param_count) const;
    bool validateParameterType(const QString& command, const QStringList& params, ParsedCommand& parsed) const;
    bool validateParameterRange(const CommandParameter& param, const QString& value, ParsedCommand& parsed) const;
    QStringList getCommandSuggestions(const QString& partial) const;
};

}  // namespace ui
}  // namespace cad
