#pragma once

#include <functional>
#include <unordered_map>

#include <QAction>
#include <QTabWidget>

namespace cad {
namespace ui {

class QtRibbon : public QTabWidget {
    Q_OBJECT

public:
    explicit QtRibbon(QWidget* parent = nullptr);
    void setCommandHandler(const std::function<void(const QString&)>& handler);

private:
    void registerDefaultActions();
    QAction* registerAction(const QString& id, const QString& label);
    QWidget* buildCommandTab(const QString& title,
                             const QList<QPair<QString, QStringList>>& groups);
    QWidget* buildGroup(const QString& name, const QStringList& command_ids);

    std::function<void(const QString&)> command_handler_;
    std::unordered_map<std::string, QAction*> actions_;
};

}  // namespace ui
}  // namespace cad
