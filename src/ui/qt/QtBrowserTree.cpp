#include "QtBrowserTree.h"

#include <QTreeWidgetItem>

namespace cad {
namespace ui {

QtBrowserTree::QtBrowserTree(QWidget* parent) : QTreeWidget(parent) {
    setHeaderHidden(true);
    root_ = new QTreeWidgetItem(this);
    root_->setText(0, tr("Model"));
    addTopLevelItem(root_);

    QTreeWidgetItem* sketches = new QTreeWidgetItem(root_);
    sketches->setText(0, tr("Sketches"));
    root_->addChild(sketches);

    QTreeWidgetItem* parts = new QTreeWidgetItem(root_);
    parts->setText(0, tr("Parts"));
    root_->addChild(parts);

    QTreeWidgetItem* assemblies = new QTreeWidgetItem(root_);
    assemblies->setText(0, tr("Assemblies"));
    root_->addChild(assemblies);

    QTreeWidgetItem* drawings = new QTreeWidgetItem(root_);
    drawings->setText(0, tr("Drawings"));
    root_->addChild(drawings);

    mates_node_ = new QTreeWidgetItem(root_);
    mates_node_->setText(0, tr("Mates"));
    root_->addChild(mates_node_);

    recent_node_ = new QTreeWidgetItem(root_);
    recent_node_->setText(0, tr("Recent Commands"));
    root_->addChild(recent_node_);

    root_->setExpanded(true);
}

void QtBrowserTree::setAssemblySummary(const QString& summary) {
    if (!root_) {
        return;
    }
    root_->setText(0, tr("Model (%1)").arg(summary));
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
    recent_node_->addChild(item);
    recent_node_->setExpanded(true);
}

}  // namespace ui
}  // namespace cad
