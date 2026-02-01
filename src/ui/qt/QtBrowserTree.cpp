#include "QtBrowserTree.h"

#include <QFile>
#include <QIcon>
#include <QMenu>
#include <QTreeWidgetItem>

namespace cad {
namespace ui {

static QIcon iconForNode(const QString& themeName, const QString& path) {
    QIcon icon = QIcon::fromTheme(themeName);
    if (icon.isNull() && !path.isEmpty() && QFile::exists(path)) {
        icon = QIcon(path);
    }
    return icon;
}

QtBrowserTree::QtBrowserTree(QWidget* parent) : QTreeWidget(parent) {
    setColumnCount(2);
    setHeaderHidden(true);
    setColumnWidth(1, 24);
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QTreeWidget::customContextMenuRequested, this, &QtBrowserTree::showContextMenu);

    QIcon eyeIcon = QIcon::fromTheme("view-visible");
    if (eyeIcon.isNull() && QFile::exists(":/icons/view/shaded.svg")) {
        eyeIcon = QIcon(":/icons/view/shaded.svg");
    }
    visibility_icon_ = eyeIcon;
    QIcon eyeOffIcon = QIcon::fromTheme("view-hidden");
    if (eyeOffIcon.isNull()) {
        eyeOffIcon = eyeIcon;  // fallback
    }
    visibility_off_icon_ = eyeOffIcon;

    root_ = new QTreeWidgetItem(this);
    root_->setText(0, tr("Model"));
    root_->setIcon(0, iconForNode("view-list-tree", ":/icons/assembly/pattern.svg"));
    addTopLevelItem(root_);

    QTreeWidgetItem* view_node = new QTreeWidgetItem(root_);
    view_node->setText(0, tr("View"));
    view_node->setIcon(0, iconForNode("view-visible", ":/icons/view/shaded.svg"));
    view_node->setIcon(1, visibility_icon_);
    root_->addChild(view_node);

    QTreeWidgetItem* origin = new QTreeWidgetItem(root_);
    origin->setText(0, tr("Origin"));
    origin->setIcon(0, iconForNode("axis", ":/icons/view/measure.svg"));
    origin->setIcon(1, visibility_icon_);
    root_->addChild(origin);
    QTreeWidgetItem* xy = new QTreeWidgetItem(origin);
    xy->setText(0, tr("XY Plane"));
    origin->addChild(xy);
    QTreeWidgetItem* yz = new QTreeWidgetItem(origin);
    yz->setText(0, tr("YZ Plane"));
    origin->addChild(yz);
    QTreeWidgetItem* xz = new QTreeWidgetItem(origin);
    xz->setText(0, tr("XZ Plane"));
    origin->addChild(xz);
    QTreeWidgetItem* axes = new QTreeWidgetItem(origin);
    axes->setText(0, tr("Axes"));
    origin->addChild(axes);

    QTreeWidgetItem* ref_geometry = new QTreeWidgetItem(root_);
    ref_geometry->setText(0, tr("Reference Geometry"));
    ref_geometry->setIcon(0, iconForNode("axis", ":/icons/view/measure.svg"));
    ref_geometry->setIcon(1, visibility_icon_);
    root_->addChild(ref_geometry);
    work_planes_node_ = new QTreeWidgetItem(ref_geometry);
    work_planes_node_->setText(0, tr("Work Planes"));
    work_planes_node_->setIcon(0, iconForNode("view-grid", ":/icons/view/section_plane.svg"));
    ref_geometry->addChild(work_planes_node_);
    work_axes_node_ = new QTreeWidgetItem(ref_geometry);
    work_axes_node_->setText(0, tr("Work Axes"));
    work_axes_node_->setIcon(0, iconForNode("axis", ":/icons/view/measure.svg"));
    ref_geometry->addChild(work_axes_node_);
    work_points_node_ = new QTreeWidgetItem(ref_geometry);
    work_points_node_->setText(0, tr("Work Points"));
    work_points_node_->setIcon(0, iconForNode("edit-pin", ":/icons/sketch/arc.svg"));
    ref_geometry->addChild(work_points_node_);

    QTreeWidgetItem* bodies = new QTreeWidgetItem(root_);
    bodies->setText(0, tr("Solid Bodies"));
    bodies->setIcon(0, iconForNode("view-3d", ":/icons/part/extrude.svg"));
    bodies->setIcon(1, visibility_icon_);
    root_->addChild(bodies);

    QTreeWidgetItem* sketches = new QTreeWidgetItem(root_);
    sketches->setText(0, tr("Sketches"));
    sketches->setIcon(0, iconForNode("draw-rectangle", ":/icons/sketch/rectangle.svg"));
    sketches->setIcon(1, visibility_icon_);
    root_->addChild(sketches);

    QTreeWidgetItem* parts = new QTreeWidgetItem(root_);
    parts->setText(0, tr("Parts"));
    parts->setIcon(0, iconForNode("view-3d", ":/icons/part/extrude.svg"));
    parts->setIcon(1, visibility_icon_);
    root_->addChild(parts);

    QTreeWidgetItem* assemblies = new QTreeWidgetItem(root_);
    assemblies->setText(0, tr("Assemblies"));
    assemblies->setIcon(0, iconForNode("folder", ":/icons/assembly/place.svg"));
    assemblies->setIcon(1, visibility_icon_);
    root_->addChild(assemblies);

    QTreeWidgetItem* drawings = new QTreeWidgetItem(root_);
    drawings->setText(0, tr("Drawings"));
    drawings->setIcon(0, iconForNode("x-office-document", ":/icons/drawing/base_view.svg"));
    drawings->setIcon(1, visibility_icon_);
    root_->addChild(drawings);

    mates_node_ = new QTreeWidgetItem(root_);
    mates_node_->setText(0, tr("Mates"));
    mates_node_->setIcon(0, iconForNode("insert-link", ":/icons/assembly/mate.svg"));
    mates_node_->setIcon(1, visibility_icon_);
    root_->addChild(mates_node_);

    recent_node_ = new QTreeWidgetItem(root_);
    recent_node_->setText(0, tr("Recent Commands"));
    recent_node_->setIcon(0, iconForNode("view-history", ":/icons/general/undo.svg"));
    recent_node_->setIcon(1, visibility_icon_);
    root_->addChild(recent_node_);

    QTreeWidgetItem* end_of_part = new QTreeWidgetItem(root_);
    end_of_part->setText(0, tr("End of Part"));
    end_of_part->setIcon(0, QIcon());
    root_->addChild(end_of_part);

    root_->setExpanded(true);
    origin->setExpanded(true);

    connect(this, &QTreeWidget::itemDoubleClicked, this, &QtBrowserTree::onItemDoubleClicked);
}

void QtBrowserTree::onItemDoubleClicked(QTreeWidgetItem* item, int column) {
    Q_UNUSED(column);
    if (!item || !command_handler_) return;
    QTreeWidgetItem* p = item->parent();
    if (p == recent_node_) {
        QString cmd = item->text(0);
        if (!cmd.isEmpty()) command_handler_(cmd);
    }
}

void QtBrowserTree::setCommandHandler(const std::function<void(const QString&)>& handler) {
    command_handler_ = handler;
}

void QtBrowserTree::showContextMenu(const QPoint& pos) {
    QTreeWidgetItem* item = itemAt(pos);
    Q_UNUSED(item);
    QMenu menu(this);
    auto run = [this](const QString& cmd) {
        if (command_handler_) command_handler_(cmd);
    };
    menu.addAction(tr("Visibility"), [run]() { run("Visibility"); });
    menu.addAction(tr("Suppress"), [run]() { run("Suppress"); });
    menu.addSeparator();
    menu.addAction(tr("Create New Component"), [run]() { run("Create New Component"); });
    menu.addAction(tr("Place From File..."), [run]() { run("Place From File"); });
    menu.addAction(tr("Edit Component"), [run]() { run("Edit Component"); });
    menu.addSeparator();
    menu.addAction(tr("Properties..."), [run]() { run("Properties"); });
    menu.addAction(tr("Rename"), [run]() { run("Rename"); });
    menu.addAction(tr("Delete"), [run]() { run("Delete"); });
    menu.addSeparator();
    menu.addAction(tr("Copy"), [run]() { run("Copy"); });
    menu.addAction(tr("Cut"), [run]() { run("Cut"); });
    menu.addAction(tr("Paste"), [run]() { run("Paste"); });
    menu.addSeparator();
    menu.addAction(tr("Search"), [run]() { run("Search"); });
    menu.addAction(tr("Show Dependencies"), [run]() { run("Show Dependencies"); });
    menu.exec(viewport()->mapToGlobal(pos));
}


void QtBrowserTree::setAssemblySummary(const QString& summary) {
    if (!root_) {
        return;
    }
    root_->setText(0, tr("Model (%1)").arg(summary));
}

void QtBrowserTree::setWorkPlanes(const QStringList& names) {
    if (!work_planes_node_) return;
    work_planes_node_->takeChildren();
    for (const QString& name : names) {
        QTreeWidgetItem* item = new QTreeWidgetItem(work_planes_node_);
        item->setText(0, name.isEmpty() ? tr("Plane") : name);
        item->setIcon(0, iconForNode("view-grid", ":/icons/view/section_plane.svg"));
        item->setIcon(1, visibility_icon_);
        work_planes_node_->addChild(item);
    }
    work_planes_node_->setExpanded(!names.isEmpty());
}

void QtBrowserTree::setWorkAxes(const QStringList& names) {
    if (!work_axes_node_) return;
    work_axes_node_->takeChildren();
    for (const QString& name : names) {
        QTreeWidgetItem* item = new QTreeWidgetItem(work_axes_node_);
        item->setText(0, name.isEmpty() ? tr("Axis") : name);
        item->setIcon(0, iconForNode("axis", ":/icons/view/measure.svg"));
        item->setIcon(1, visibility_icon_);
        work_axes_node_->addChild(item);
    }
    work_axes_node_->setExpanded(!names.isEmpty());
}

void QtBrowserTree::setWorkPoints(const QStringList& names) {
    if (!work_points_node_) return;
    work_points_node_->takeChildren();
    for (const QString& name : names) {
        QTreeWidgetItem* item = new QTreeWidgetItem(work_points_node_);
        item->setText(0, name.isEmpty() ? tr("Point") : name);
        item->setIcon(0, iconForNode("edit-pin", ":/icons/sketch/arc.svg"));
        item->setIcon(1, visibility_icon_);
        work_points_node_->addChild(item);
    }
    work_points_node_->setExpanded(!names.isEmpty());
}

void QtBrowserTree::setMatesSummary(const QString& summary) {
    if (!mates_node_) {
        return;
    }
    mates_node_->setText(0, tr("Mates (%1)").arg(summary));
}

void QtBrowserTree::appendRecentCommand(const QString& command) {
    if (!recent_node_) {
        return;
    }
    QTreeWidgetItem* item = new QTreeWidgetItem(recent_node_);
    item->setText(0, command);
    item->setIcon(1, visibility_icon_);
    recent_node_->addChild(item);
    recent_node_->setExpanded(true);
}

}  // namespace ui
}  // namespace cad
