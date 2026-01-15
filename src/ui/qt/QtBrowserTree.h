#pragma once

#include <QTreeWidget>

namespace cad {
namespace ui {

class QtBrowserTree : public QTreeWidget {
    Q_OBJECT

public:
    explicit QtBrowserTree(QWidget* parent = nullptr);
    void setAssemblySummary(const QString& summary);
    void setMatesSummary(const QString& summary);
    void appendRecentCommand(const QString& command);

private:
    QTreeWidgetItem* root_{nullptr};
    QTreeWidgetItem* mates_node_{nullptr};
    QTreeWidgetItem* recent_node_{nullptr};
};

}  // namespace ui
}  // namespace cad
