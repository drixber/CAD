#include "QtRegisterDialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>

namespace cad {
namespace ui {

QtRegisterDialog::QtRegisterDialog(QWidget* parent)
    : QDialog(parent) {
    setWindowTitle(tr("Hydra CAD - Register"));
    setModal(true);
    setMinimumWidth(400);
    
    QVBoxLayout* main_layout = new QVBoxLayout(this);
    
    QLabel* title_label = new QLabel(tr("Create Account"), this);
    title_label->setStyleSheet("font-size: 18px; font-weight: bold;");
    main_layout->addWidget(title_label);
    
    QLabel* subtitle_label = new QLabel(tr("Create a new account to get started"), this);
    subtitle_label->setStyleSheet("color: gray;");
    main_layout->addWidget(subtitle_label);
    
    main_layout->addSpacing(20);
    
    QFormLayout* form_layout = new QFormLayout();
    
    username_edit_ = new QLineEdit(this);
    username_edit_->setPlaceholderText(tr("At least 3 characters"));
    form_layout->addRow(tr("Username:"), username_edit_);
    
    email_edit_ = new QLineEdit(this);
    email_edit_->setPlaceholderText(tr("your.email@example.com"));
    form_layout->addRow(tr("Email:"), email_edit_);
    
    password_edit_ = new QLineEdit(this);
    password_edit_->setPlaceholderText(tr("Min 8 characters, letters and numbers"));
    password_edit_->setEchoMode(QLineEdit::Password);
    form_layout->addRow(tr("Password:"), password_edit_);
    
    confirm_password_edit_ = new QLineEdit(this);
    confirm_password_edit_->setPlaceholderText(tr("Confirm password"));
    confirm_password_edit_->setEchoMode(QLineEdit::Password);
    form_layout->addRow(tr("Confirm:"), confirm_password_edit_);
    
    main_layout->addLayout(form_layout);
    
    error_label_ = new QLabel(this);
    error_label_->setStyleSheet("color: red;");
    error_label_->setWordWrap(true);
    error_label_->hide();
    main_layout->addWidget(error_label_);
    
    main_layout->addSpacing(10);
    
    QHBoxLayout* button_layout = new QHBoxLayout();
    button_layout->addStretch();
    
    QPushButton* cancel_button = new QPushButton(tr("Cancel"), this);
    button_layout->addWidget(cancel_button);
    
    register_button_ = new QPushButton(tr("Register"), this);
    register_button_->setDefault(true);
    register_button_->setEnabled(false);
    button_layout->addWidget(register_button_);
    
    main_layout->addLayout(button_layout);
    
    connect(register_button_, &QPushButton::clicked, this, &QtRegisterDialog::onRegisterClicked);
    connect(cancel_button, &QPushButton::clicked, this, &QDialog::reject);
    connect(username_edit_, &QLineEdit::textChanged, this, &QtRegisterDialog::onTextChanged);
    connect(email_edit_, &QLineEdit::textChanged, this, &QtRegisterDialog::onTextChanged);
    connect(password_edit_, &QLineEdit::textChanged, this, &QtRegisterDialog::onTextChanged);
    connect(confirm_password_edit_, &QLineEdit::textChanged, this, &QtRegisterDialog::onTextChanged);
    connect(confirm_password_edit_, &QLineEdit::returnPressed, this, &QtRegisterDialog::onRegisterClicked);
}

QString QtRegisterDialog::getUsername() const {
    return username_edit_->text();
}

QString QtRegisterDialog::getEmail() const {
    return email_edit_->text();
}

QString QtRegisterDialog::getPassword() const {
    return password_edit_->text();
}

QString QtRegisterDialog::getConfirmPassword() const {
    return confirm_password_edit_->text();
}

void QtRegisterDialog::setError(const QString& error_message) {
    error_label_->setText(error_message);
    error_label_->show();
}

void QtRegisterDialog::onRegisterClicked() {
    QString username = username_edit_->text().trimmed();
    QString email = email_edit_->text().trimmed();
    QString password = password_edit_->text();
    QString confirm = confirm_password_edit_->text();
    
    if (password != confirm) {
        setError(tr("Passwords do not match"));
        return;
    }
    
    emit registerRequested(username, email, password, confirm);
}

void QtRegisterDialog::onTextChanged() {
    bool has_all_fields = !username_edit_->text().trimmed().isEmpty() &&
                         !email_edit_->text().trimmed().isEmpty() &&
                         !password_edit_->text().isEmpty() &&
                         !confirm_password_edit_->text().isEmpty();
    register_button_->setEnabled(has_all_fields);
    
    if (has_all_fields && error_label_->isVisible()) {
        error_label_->hide();
    }
}

}  // namespace ui
}  // namespace cad
