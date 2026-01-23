#include "QtUpdateDialog.h"

namespace cad {
namespace ui {

QtUpdateDialog::QtUpdateDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle(tr("Update Available"));
    setModal(true);
    setMinimumWidth(500);
    setupUI();
}

void QtUpdateDialog::setupUI() {
    QVBoxLayout* main_layout = new QVBoxLayout(this);
    main_layout->setSpacing(12);
    main_layout->setContentsMargins(16, 16, 16, 16);
    
    // Title
    QLabel* title_label = new QLabel(tr("Update Available"), this);
    title_label->setStyleSheet("font-size: 18px; font-weight: bold; color: #2B78E4;");
    main_layout->addWidget(title_label);
    
    // Version
    version_label_ = new QLabel(this);
    version_label_->setStyleSheet("font-size: 14px; font-weight: 600;");
    main_layout->addWidget(version_label_);
    
    // Changelog
    QLabel* changelog_label = new QLabel(tr("What's New:"), this);
    changelog_label->setStyleSheet("font-weight: 600;");
    main_layout->addWidget(changelog_label);
    
    changelog_edit_ = new QTextEdit(this);
    changelog_edit_->setReadOnly(true);
    changelog_edit_->setMaximumHeight(150);
    changelog_edit_->setStyleSheet(
        "QTextEdit {"
        "    background: #F5F5F5;"
        "    border: 1px solid #D0D0D0;"
        "    border-radius: 4px;"
        "    padding: 8px;"
        "    font-size: 12px;"
        "}"
    );
    main_layout->addWidget(changelog_edit_);
    
    // Progress (initially hidden)
    status_label_ = new QLabel(this);
    status_label_->setStyleSheet("color: #757575; font-size: 12px;");
    status_label_->hide();
    main_layout->addWidget(status_label_);
    
    progress_bar_ = new QProgressBar(this);
    progress_bar_->setRange(0, 100);
    progress_bar_->setValue(0);
    progress_bar_->setTextVisible(true);
    progress_bar_->hide();
    main_layout->addWidget(progress_bar_);
    
    current_file_label_ = new QLabel(this);
    current_file_label_->setStyleSheet("color: #757575; font-size: 11px;");
    current_file_label_->hide();
    main_layout->addWidget(current_file_label_);
    
    // Options
    auto_install_check_ = new QCheckBox(tr("Install automatically when download completes"), this);
    auto_install_check_->setChecked(true);
    main_layout->addWidget(auto_install_check_);
    
    create_backup_check_ = new QCheckBox(tr("Create backup before installing (recommended)"), this);
    create_backup_check_->setChecked(true);
    main_layout->addWidget(create_backup_check_);
    
    main_layout->addStretch();
    
    // Buttons
    QHBoxLayout* button_layout = new QHBoxLayout();
    button_layout->addStretch();
    
    if (!mandatory_) {
        skip_button_ = new QPushButton(tr("Skip This Version"), this);
        skip_button_->setStyleSheet(
            "QPushButton {"
            "    background: transparent;"
            "    color: #757575;"
            "    border: 1px solid #D0D0D0;"
            "    border-radius: 4px;"
            "    padding: 8px 16px;"
            "}"
            "QPushButton:hover {"
            "    background: #F5F5F5;"
            "}"
        );
        button_layout->addWidget(skip_button_);
        connect(skip_button_, &QPushButton::clicked, this, &QtUpdateDialog::onSkipClicked);
    }
    
    later_button_ = new QPushButton(tr("Later"), this);
    later_button_->setStyleSheet(
        "QPushButton {"
        "    background: transparent;"
        "    color: #757575;"
        "    border: 1px solid #D0D0D0;"
        "    border-radius: 4px;"
        "    padding: 8px 16px;"
        "}"
        "QPushButton:hover {"
        "    background: #F5F5F5;"
        "}"
    );
    button_layout->addWidget(later_button_);
    connect(later_button_, &QPushButton::clicked, this, &QtUpdateDialog::onLaterClicked);
    
    install_button_ = new QPushButton(tr("Install Update"), this);
    install_button_->setStyleSheet(
        "QPushButton {"
        "    background: #2B78E4;"
        "    color: #FFFFFF;"
        "    border: none;"
        "    border-radius: 4px;"
        "    padding: 8px 16px;"
        "    font-weight: 600;"
        "    min-width: 120px;"
        "}"
        "QPushButton:hover {"
        "    background: #1E5AA8;"
        "}"
    );
    button_layout->addWidget(install_button_);
    connect(install_button_, &QPushButton::clicked, this, &QtUpdateDialog::onInstallClicked);
    
    main_layout->addLayout(button_layout);
}

void QtUpdateDialog::setUpdateInfo(const QString& version, const QString& changelog, bool mandatory) {
    mandatory_ = mandatory;
    version_label_->setText(tr("Version %1 is available").arg(version));
    changelog_edit_->setPlainText(changelog);
    
    if (mandatory) {
        setWindowTitle(tr("Mandatory Update Available"));
        if (skip_button_) {
            skip_button_->hide();
        }
        later_button_->setText(tr("Restart Later"));
    }
}

void QtUpdateDialog::setProgress(int percentage, const QString& status) {
    status_label_->setText(status);
    status_label_->show();
    progress_bar_->setValue(percentage);
    progress_bar_->show();
    
    // Disable buttons during installation
    install_button_->setEnabled(false);
    later_button_->setEnabled(false);
    if (skip_button_) {
        skip_button_->setEnabled(false);
    }
}

void QtUpdateDialog::setCurrentFile(const QString& file) {
    if (!file.isEmpty()) {
        current_file_label_->setText(tr("Updating: %1").arg(file));
        current_file_label_->show();
    } else {
        current_file_label_->hide();
    }
}

void QtUpdateDialog::setCompleted(bool success, const QString& message) {
    progress_bar_->setValue(100);
    
    if (success) {
        status_label_->setText(tr("Update installed successfully!"));
        status_label_->setStyleSheet("color: #4CAF50; font-weight: 600;");
        
        install_button_->setText(tr("Restart Now"));
        install_button_->setEnabled(true);
        connect(install_button_, &QPushButton::clicked, this, &QDialog::accept);
    } else {
        status_label_->setText(tr("Update failed: %1").arg(message));
        status_label_->setStyleSheet("color: #F44336; font-weight: 600;");
        
        install_button_->setText(tr("Retry"));
        install_button_->setEnabled(true);
    }
}

bool QtUpdateDialog::getAutoInstall() const {
    return auto_install_check_->isChecked();
}

bool QtUpdateDialog::getCreateBackup() const {
    return create_backup_check_->isChecked();
}

void QtUpdateDialog::onInstallClicked() {
    emit installRequested();
}

void QtUpdateDialog::onLaterClicked() {
    emit laterRequested();
    reject();
}

void QtUpdateDialog::onSkipClicked() {
    emit skipRequested();
    reject();
}

}  // namespace ui
}  // namespace cad
