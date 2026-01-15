#include "QtBrowserTree.h"

#include <QTreeWidgetItem>

namespace cad {
namespace ui {

QtBrowserTree::QtBrowserTree(QWidget* parent) : QTreeWidget(parent) {
    setHeaderHidden(true);
    root_ = new QTreeWidgetItem(this);
    root_->setText(0, tr("Model"));
    addTopLevelItem(root_);
    mates_node_ = new QTreeWidgetItem(root_);
    mates_node_->setText(0, tr("Mates"));
    root_->addChild(mates_node_);
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

}  // namespace ui
}  // namespace cad
