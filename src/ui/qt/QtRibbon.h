#pragma once

#include <functional>
#include <unordered_map>

#include <QAction>
#include <QIcon>
#include <QLineEdit>
#include <QTabBar>
#include <QTabWidget>
#include <QWidget>

namespace cad {
namespace ui {

class QtRibbon : public QTabWidget {
    Q_OBJECT

public:
    explicit QtRibbon(QWidget* parent = nullptr);
    void setCommandHandler(const std::function<void(const QString&)>& handler);
    /** Returns the search line edit (placeholder "Search Help & Commands…") for integration in layout. */
    QLineEdit* searchLineEdit() const { return search_line_; }
    /** Returns the document tab bar (open documents) for integration below ribbon. */
    QTabBar* documentTabBar() const { return document_tabs_; }

private:
    void registerDefaultActions();
    QAction* registerAction(const QString& id, const QString& label);
    QWidget* buildCommandTab(const QString& title,
                             const QList<QPair<QString, QStringList>>& groups);
    QWidget* buildGroup(const QString& name, const QStringList& command_ids);
    QIcon getIcon(const QString& commandId);
    QString getIconPath(const QString& commandId);

    std::function<void(const QString&)> command_handler_;
    std::unordered_map<std::string, QAction*> actions_;
    QLineEdit* search_line_{nullptr};
    QTabBar* document_tabs_{nullptr};
};

}  // namespace ui
}  // namespace cad
