#include "QtPrintersDialog.h"
#include "../app/PrinterService.h"

#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QListWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QApplication>

namespace cad {
namespace ui {

QtPrintersDialog::QtPrintersDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle(tr("3D Printers"));
    setMinimumSize(480, 320);
    QHBoxLayout* main = new QHBoxLayout(this);

    list_ = new QListWidget(this);
    list_->setMinimumWidth(160);
    main->addWidget(list_);

    QVBoxLayout* right = new QVBoxLayout();
    QGroupBox* form_group = new QGroupBox(tr("Printer"), this);
    QFormLayout* form = new QFormLayout(form_group);
    name_edit_ = new QLineEdit(this);
    name_edit_->setPlaceholderText(tr("e.g. My Prusa"));
    form->addRow(tr("Name:"), name_edit_);
    type_combo_ = new QComboBox(this);
    type_combo_->addItem(tr("OctoPrint"), "octoprint");
    type_combo_->addItem(tr("Moonraker (Klipper)"), "moonraker");
    form->addRow(tr("Type:"), type_combo_);
    url_edit_ = new QLineEdit(this);
    url_edit_->setPlaceholderText(tr("http://192.168.1.100:5000"));
    form->addRow(tr("URL:"), url_edit_);
    api_key_edit_ = new QLineEdit(this);
    api_key_edit_->setPlaceholderText(tr("API key (optional)"));
    api_key_edit_->setEchoMode(QLineEdit::Password);
    form->addRow(tr("API Key:"), api_key_edit_);

    form_container_ = new QWidget(this);
    QVBoxLayout* form_container_layout = new QVBoxLayout(form_container_);
    form_container_layout->addWidget(form_group);

    status_label_ = new QLabel(this);
    status_label_->setWordWrap(true);
    form_container_layout->addWidget(status_label_);

    QHBoxLayout* btns = new QHBoxLayout();
    QPushButton* add_btn = new QPushButton(tr("Add"), this);
    save_btn_ = new QPushButton(tr("Save"), this);
    remove_btn_ = new QPushButton(tr("Remove"), this);
    test_btn_ = new QPushButton(tr("Test connection"), this);
    send_to_btn_ = new QPushButton(tr("Send current model to selected printer"), this);
    send_to_btn_->setVisible(false);
    btns->addWidget(add_btn);
    btns->addWidget(save_btn_);
    btns->addWidget(remove_btn_);
    btns->addWidget(test_btn_);
    btns->addStretch();
    form_container_layout->addLayout(btns);

    right->addWidget(form_container_);
    QPushButton* close_btn = new QPushButton(tr("Close"), this);
    right->addWidget(send_to_btn_);
    send_to_btn_->setVisible(false);
    right->addStretch();
    right->addWidget(close_btn, 0, Qt::AlignRight);
    main->addLayout(right, 1);

    connect(list_, &QListWidget::currentRowChanged, this, [this](int) { onPrinterSelected(); });
    connect(add_btn, &QPushButton::clicked, this, &QtPrintersDialog::onAddClicked);
    connect(save_btn_, &QPushButton::clicked, this, &QtPrintersDialog::onSaveClicked);
    connect(remove_btn_, &QPushButton::clicked, this, &QtPrintersDialog::onRemoveClicked);
    connect(test_btn_, &QPushButton::clicked, this, &QtPrintersDialog::onTestClicked);
    connect(close_btn, &QPushButton::clicked, this, &QDialog::accept);
    connect(send_to_btn_, &QPushButton::clicked, this, [this]() {
        if (!selectedPrinterId().empty()) accept();
    });

    clearForm();
}

void QtPrintersDialog::setSendMode(bool send_only) {
    send_only_mode_ = send_only;
    form_container_->setVisible(!send_only);
    send_to_btn_->setVisible(send_only);
    if (send_only) {
        setWindowTitle(tr("Send to 3D Printer"));
    } else {
        setWindowTitle(tr("3D Printers"));
    }
}

void QtPrintersDialog::setPrinterService(cad::app::PrinterService* service) {
    printer_service_ = service;
    refreshList();
}

std::string QtPrintersDialog::selectedPrinterId() const {
    QListWidgetItem* item = list_->currentItem();
    if (!item || !item->data(Qt::UserRole).isValid()) return {};
    return item->data(Qt::UserRole).toString().toStdString();
}

void QtPrintersDialog::refreshList() {
    list_->clear();
    current_id_.clear();
    if (!printer_service_) return;
    for (const auto& p : printer_service_->getPrinters()) {
        QListWidgetItem* item = new QListWidgetItem(QString::fromStdString(p.name));
        item->setData(Qt::UserRole, QString::fromStdString(p.id));
        list_->addItem(item);
    }
    clearForm();
}

void QtPrintersDialog::loadToForm(const std::string& id) {
    if (!printer_service_ || id.empty()) return;
    cad::app::PrinterProfile p = printer_service_->getPrinter(id);
    if (p.id.empty()) return;
    current_id_ = id;
    name_edit_->setText(QString::fromStdString(p.name));
    int idx = type_combo_->findData(QString::fromStdString(p.type));
    if (idx >= 0) type_combo_->setCurrentIndex(idx);
    url_edit_->setText(QString::fromStdString(p.base_url));
    api_key_edit_->setText(QString::fromStdString(p.api_key));
    remove_btn_->setEnabled(true);
    test_btn_->setEnabled(true);
    save_btn_->setEnabled(true);
    status_label_->clear();
}

void QtPrintersDialog::clearForm() {
    current_id_.clear();
    name_edit_->clear();
    type_combo_->setCurrentIndex(0);
    url_edit_->clear();
    api_key_edit_->clear();
    status_label_->clear();
    remove_btn_->setEnabled(false);
    test_btn_->setEnabled(false);
    save_btn_->setEnabled(false);
}

void QtPrintersDialog::onPrinterSelected() {
    std::string id = selectedPrinterId();
    if (id.empty()) {
        clearForm();
        return;
    }
    loadToForm(id);
}

void QtPrintersDialog::onAddClicked() {
    clearForm();
    current_id_.clear();
    name_edit_->setFocus();
    save_btn_->setEnabled(true);
    status_label_->setText(tr("Enter details and click Save to add a new printer."));
}

void QtPrintersDialog::onSaveClicked() {
    if (!printer_service_) return;
    std::string name = name_edit_->text().trimmed().toStdString();
    std::string url = url_edit_->text().trimmed().toStdString();
    if (name.empty() || url.empty()) {
        status_label_->setText(tr("Name and URL are required."));
        return;
    }
    cad::app::PrinterProfile p;
    p.name = name;
    p.type = type_combo_->currentData().toString().toStdString();
    if (p.type.empty()) p.type = "octoprint";
    p.base_url = url;
    p.api_key = api_key_edit_->text().trimmed().toStdString();
    if (current_id_.empty()) {
        printer_service_->addPrinter(p);
        status_label_->setText(tr("Printer added."));
    } else {
        p.id = current_id_;
        printer_service_->updatePrinter(current_id_, p);
        status_label_->setText(tr("Printer updated."));
    }
    refreshList();
    if (current_id_.empty() && list_->count() > 0) {
        list_->setCurrentRow(list_->count() - 1);
    } else if (!current_id_.empty()) {
        for (int i = 0; i < list_->count(); ++i) {
            if (list_->item(i)->data(Qt::UserRole).toString().toStdString() == current_id_) {
                list_->setCurrentRow(i);
                break;
            }
        }
    }
}

void QtPrintersDialog::onRemoveClicked() {
    if (current_id_.empty() || !printer_service_) return;
    if (QMessageBox::question(this, tr("Remove printer"),
                              tr("Remove this printer from the list?"),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes)
        return;
    printer_service_->removePrinter(current_id_);
    refreshList();
}

void QtPrintersDialog::onTestClicked() {
    if (current_id_.empty() || !printer_service_) return;
    test_btn_->setEnabled(false);
    status_label_->setText(tr("Testing connection…"));
    QApplication::processEvents();
    auto result = printer_service_->testConnection(current_id_);
    test_btn_->setEnabled(true);
    if (result.success) {
        status_label_->setText(tr("Connection OK."));
    } else {
        status_label_->setText(tr("Connection failed: %1").arg(QString::fromStdString(result.message)));
    }
}

}  // namespace ui
}  // namespace cad
