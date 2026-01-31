#include "QtBrowserTree.h"

#include <QMenu>
#include <QTreeWidgetItem>

namespace cad {
namespace ui {

QtBrowserTree::QtBrowserTree(QWidget* parent) : QTreeWidget(parent) {
    setHeaderHidden(true);
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QTreeWidget::customContextMenuRequested, this, &QtBrowserTree::showContextMenu);

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

void QtBrowserTree::showContextMenu(const QPoint& pos) {
    QTreeWidgetItem* item = itemAt(pos);
    QMenu menu(this);
    menu.addAction(tr("Visibility"), []() {});
    menu.addAction(tr("Suppress"), []() {});
    menu.addSeparator();
    menu.addAction(tr("Create New Component"), []() {});
    menu.addAction(tr("Place From File..."), []() {});
    menu.addAction(tr("Edit Component"), []() {});
    menu.addSeparator();
    menu.addAction(tr("Properties..."), []() {});
    menu.addAction(tr("Rename"), []() {});
    menu.addAction(tr("Delete"), []() {});
    menu.addSeparator();
    menu.addAction(tr("Copy"), []() {});
    menu.addAction(tr("Cut"), []() {});
    menu.addAction(tr("Paste"), []() {});
    menu.addSeparator();
    menu.addAction(tr("Search"), []() {});
    menu.addAction(tr("Show Dependencies"), []() {});
    menu.exec(viewport()->mapToGlobal(pos));
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
