#include "QtLoginDialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QMessageBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>

namespace cad {
namespace ui {

QtLoginDialog::QtLoginDialog(QWidget* parent)
    : QDialog(parent) {
    setWindowTitle(tr("Hydra CAD - Login"));
    setModal(true);
    setMinimumWidth(350);
    
    QVBoxLayout* main_layout = new QVBoxLayout(this);
    
    QLabel* title_label = new QLabel(tr("Welcome to Hydra CAD"), this);
    title_label->setStyleSheet("font-size: 18px; font-weight: bold;");
    main_layout->addWidget(title_label);
    
    QLabel* subtitle_label = new QLabel(tr("Please login to continue"), this);
    subtitle_label->setStyleSheet("color: gray;");
    main_layout->addWidget(subtitle_label);
    
    main_layout->addSpacing(20);
    
    QFormLayout* form_layout = new QFormLayout();
    
    username_edit_ = new QLineEdit(this);
    username_edit_->setPlaceholderText(tr("Username"));
    form_layout->addRow(tr("Username:"), username_edit_);
    
    password_edit_ = new QLineEdit(this);
    password_edit_->setPlaceholderText(tr("Password"));
    password_edit_->setEchoMode(QLineEdit::Password);
    form_layout->addRow(tr("Password:"), password_edit_);
    
    main_layout->addLayout(form_layout);
    
    remember_me_checkbox_ = new QCheckBox(tr("Remember me"), this);
    main_layout->addWidget(remember_me_checkbox_);
    
    error_label_ = new QLabel(this);
    error_label_->setStyleSheet("color: red;");
    error_label_->setWordWrap(true);
    error_label_->hide();
    main_layout->addWidget(error_label_);
    
    main_layout->addSpacing(10);
    
    QHBoxLayout* button_layout = new QHBoxLayout();
    
    register_button_ = new QPushButton(tr("Register"), this);
    button_layout->addWidget(register_button_);
    
    button_layout->addStretch();
    
    login_button_ = new QPushButton(tr("Login"), this);
    login_button_->setDefault(true);
    login_button_->setEnabled(false);
    button_layout->addWidget(login_button_);
    
    main_layout->addLayout(button_layout);
    
    connect(login_button_, &QPushButton::clicked, this, &QtLoginDialog::onLoginClicked);
    connect(register_button_, &QPushButton::clicked, this, &QtLoginDialog::onRegisterClicked);
    connect(username_edit_, &QLineEdit::textChanged, this, &QtLoginDialog::onTextChanged);
    connect(password_edit_, &QLineEdit::textChanged, this, &QtLoginDialog::onTextChanged);
    connect(password_edit_, &QLineEdit::returnPressed, this, &QtLoginDialog::onLoginClicked);
}

QString QtLoginDialog::getUsername() const {
    return username_edit_->text();
}

QString QtLoginDialog::getPassword() const {
    return password_edit_->text();
}

bool QtLoginDialog::getRememberMe() const {
    return remember_me_checkbox_->isChecked();
}

bool QtLoginDialog::shouldShowRegister() const {
    return false;  // Will be set by signal
}

void QtLoginDialog::setError(const QString& error_message) {
    error_label_->setText(error_message);
    error_label_->show();
}

void QtLoginDialog::setRememberedUsername(const QString& username) {
    username_edit_->setText(username);
    if (!username.isEmpty()) {
        password_edit_->setFocus();
    }
}

void QtLoginDialog::onLoginClicked() {
    QString username = username_edit_->text().trimmed();
    QString password = password_edit_->text();
    
    if (username.isEmpty() || password.isEmpty()) {
        setError(tr("Please enter username and password"));
        return;
    }
    
    emit loginRequested(username, password, remember_me_checkbox_->isChecked());
}

void QtLoginDialog::onRegisterClicked() {
    emit registerRequested();
}

void QtLoginDialog::onTextChanged() {
    bool has_text = !username_edit_->text().trimmed().isEmpty() && 
                    !password_edit_->text().isEmpty();
    login_button_->setEnabled(has_text);
    
    if (has_text && error_label_->isVisible()) {
        error_label_->hide();
    }
}

}  // namespace ui
}  // namespace cad
