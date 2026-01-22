#pragma once

#include <QLineEdit>
#include <QStringList>

namespace cad {
namespace ui {

struct ParsedCommand {
    QString command;
    QStringList parameters;
    bool valid{true};
    QString error_message;
};

class QtCommandLine : public QLineEdit {
    Q_OBJECT

public:
    explicit QtCommandLine(QWidget* parent = nullptr);
    void setHistory(const QStringList& history);
    const QStringList& history() const;
    QString takeCurrentCommand();
    ParsedCommand parseCommand(const QString& input) const;
    bool validateCommand(const ParsedCommand& parsed) const;

signals:
    void commandParsed(const ParsedCommand& parsed);

protected:
    void keyPressEvent(QKeyEvent* event) override;

private:
    QStringList history_{};
    int history_index_{-1};
    QStringList valid_commands_;
    
    void initializeValidCommands();
    bool isValidCommandName(const QString& name) const;
    bool validateParameterCount(const QString& command, int param_count) const;
    bool validateParameterType(const QString& command, const QStringList& params) const;
};

}  // namespace ui
}  // namespace cad
