#include "QtMateDialog.h"
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QLabel>

namespace cad {
namespace ui {

QtMateDialog::QtMateDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle(tr("Mate Constraint"));
    auto* layout = new QVBoxLayout(this);

    auto* form = new QFormLayout();
    type_combo_ = new QComboBox(this);
    type_combo_->addItem(tr("Coincident"), QString("Coincident"));
    type_combo_->addItem(tr("Parallel"), QString("Parallel"));
    type_combo_->addItem(tr("Perpendicular"), QString("Perpendicular"));
    type_combo_->addItem(tr("Tangent"), QString("Tangent"));
    type_combo_->addItem(tr("Concentric"), QString("Concentric"));
    type_combo_->addItem(tr("Distance"), QString("Distance"));
    form->addRow(tr("Mate type:"), type_combo_);

    value_check_ = new QCheckBox(tr("Use value (distance or angle)"), this);
    value_check_->setChecked(false);
    form->addRow(value_check_);

    value_spin_ = new QDoubleSpinBox(this);
    value_spin_->setRange(-1e6, 1e6);
    value_spin_->setDecimals(2);
    value_spin_->setValue(10.0);
    value_spin_->setSuffix(tr(" mm"));
    value_spin_->setEnabled(false);
    form->addRow(tr("Value:"), value_spin_);

    connect(value_check_, &QCheckBox::toggled, value_spin_, &QDoubleSpinBox::setEnabled);
    connect(type_combo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int) { onTypeChanged(type_combo_->currentData().toString()); });

    layout->addLayout(form);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
}

QString QtMateDialog::mateType() const {
    return type_combo_->currentData().toString();
}

double QtMateDialog::mateValue() const {
    return value_spin_->value();
}

bool QtMateDialog::isValueUsed() const {
    return value_check_->isChecked();
}

void QtMateDialog::onTypeChanged(const QString& type) {
    if (type == "Distance" || type == "Parallel") {
        value_spin_->setSuffix(tr(" mm"));
    } else if (type == "Perpendicular") {
        value_spin_->setSuffix(tr(" °"));
        value_spin_->setValue(90.0);
    } else {
        value_spin_->setSuffix(tr(" mm"));
    }
}

}  // namespace ui
}  // namespace cad
