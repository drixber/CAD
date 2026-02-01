#pragma once

#include <QTreeWidget>
#include <functional>

namespace cad {
namespace ui {

class QtBrowserTree : public QTreeWidget {
    Q_OBJECT

public:
    explicit QtBrowserTree(QWidget* parent = nullptr);
    void setAssemblySummary(const QString& summary);
    void setMatesSummary(const QString& summary);
    void appendRecentCommand(const QString& command);
    void setCommandHandler(const std::function<void(const QString&)>& handler);
    /** Update reference geometry (Arbeitselemente) from current part. */
    void setWorkPlanes(const QStringList& names);
    void setWorkAxes(const QStringList& names);
    void setWorkPoints(const QStringList& names);

private slots:
    void showContextMenu(const QPoint& pos);
    void onItemDoubleClicked(QTreeWidgetItem* item, int column);

private:
    std::function<void(const QString&)> command_handler_;
    QTreeWidgetItem* root_{nullptr};
    QTreeWidgetItem* mates_node_{nullptr};
    QTreeWidgetItem* recent_node_{nullptr};
    QTreeWidgetItem* work_planes_node_{nullptr};
    QTreeWidgetItem* work_axes_node_{nullptr};
    QTreeWidgetItem* work_points_node_{nullptr};
    QIcon visibility_icon_;
    QIcon visibility_off_icon_;
};

}  // namespace ui
}  // namespace cad
