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
    /** Filter tree by text (case-insensitive); empty string shows all. */
    void setFilterText(const QString& text);

signals:
    /** Emitted when the user selects an item (single click). breadcrumb = path like "Model > Sketches", nodeName = selected item text. */
    void nodeSelectionChanged(const QString& breadcrumb, const QString& nodeName);

private slots:
    void showContextMenu(const QPoint& pos);
    void onItemDoubleClicked(QTreeWidgetItem* item, int column);
    void onSelectionChanged();

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
