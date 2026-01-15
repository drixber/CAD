#include "QtPropertyPanel.h"

#include <QLabel>
#include <QVBoxLayout>

namespace cad {
namespace ui {

QtPropertyPanel::QtPropertyPanel(QWidget* parent) : QWidget(parent) {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel(tr("Selection")));
    constraints_label_ = new QLabel(tr("Constraints: 0"));
    layout->addWidget(constraints_label_);
    parameters_label_ = new QLabel(tr("Parameters: 0"));
    layout->addWidget(parameters_label_);
    parameters_summary_ = new QLabel(tr("Parameter Summary:"));
    parameters_summary_->setWordWrap(true);
    layout->addWidget(parameters_summary_);
    integration_status_ = new QLabel(tr("Integration: FreeCAD off"));
    layout->addWidget(integration_status_);
    mates_label_ = new QLabel(tr("Mates: 0"));
    layout->addWidget(mates_label_);
    layout->addStretch();
}

void QtPropertyPanel::setConstraintCount(int count) {
    if (constraints_label_) {
        constraints_label_->setText(tr("Constraints: %1").arg(count));
    }
}

void QtPropertyPanel::setParameterCount(int count) {
    if (parameters_label_) {
        parameters_label_->setText(tr("Parameters: %1").arg(count));
    }
}

void QtPropertyPanel::setParameterSummary(const QString& summary) {
    if (parameters_summary_) {
        parameters_summary_->setText(tr("Parameter Summary: %1").arg(summary));
    }
}

void QtPropertyPanel::setIntegrationStatus(const QString& status) {
    if (integration_status_) {
        integration_status_->setText(tr("Integration: %1").arg(status));
    }
}

void QtPropertyPanel::setMateCount(int count) {
    if (mates_label_) {
        mates_label_->setText(tr("Mates: %1").arg(count));
    }
}

}  // namespace ui
}  // namespace cad
