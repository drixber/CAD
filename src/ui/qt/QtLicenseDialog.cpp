#include "QtLicenseDialog.h"
#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>

namespace cad {
namespace ui {

QtLicenseDialog::QtLicenseDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle(tr("Activate License"));
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel(tr("Enter your license key:")));
    key_edit_ = new QLineEdit(this);
    key_edit_->setPlaceholderText(tr("License key"));
    key_edit_->setMinimumWidth(320);
    layout->addWidget(key_edit_);
    status_label_ = new QLabel(this);
    status_label_->setWordWrap(true);
    layout->addWidget(status_label_);
    QHBoxLayout* btn_layout = new QHBoxLayout();
    btn_layout->addStretch();
    activate_btn_ = new QPushButton(tr("Activate"), this);
    QPushButton* cancel_btn = new QPushButton(tr("Cancel"), this);
    connect(activate_btn_, &QPushButton::clicked, this, &QtLicenseDialog::onActivateClicked);
    connect(cancel_btn, &QPushButton::clicked, this, &QDialog::reject);
    btn_layout->addWidget(activate_btn_);
    btn_layout->addWidget(cancel_btn);
    layout->addLayout(btn_layout);
}

void QtLicenseDialog::setActivateCallback(std::function<bool(const std::string&)> callback) {
    activate_callback_ = std::move(callback);
}

void QtLicenseDialog::onActivateClicked() {
    std::string key = key_edit_->text().trimmed().toStdString();
    if (key.empty()) {
        status_label_->setText(tr("Please enter a license key."));
        return;
    }
    if (!activate_callback_) {
        status_label_->setText(tr("Not configured."));
        return;
    }
    activate_btn_->setEnabled(false);
    status_label_->setText(tr("Activating…"));
    QApplication::processEvents();
    bool ok = activate_callback_(key);
    activate_btn_->setEnabled(true);
    if (ok) {
        QMessageBox::information(this, tr("License"), tr("License activated successfully."));
        accept();
    } else {
        status_label_->setText(tr("Activation failed. Check the key or try again later."));
    }
}

}  // namespace ui
}  // namespace cad
