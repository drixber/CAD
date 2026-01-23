#pragma once

#include <QDialog>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCheckBox>

namespace cad {
namespace ui {

class QtUpdateDialog : public QDialog {
    Q_OBJECT

public:
    explicit QtUpdateDialog(QWidget* parent = nullptr);
    ~QtUpdateDialog() override = default;
    
    // Update info
    void setUpdateInfo(const QString& version, const QString& changelog, bool mandatory);
    
    // Progress
    void setProgress(int percentage, const QString& status);
    void setCurrentFile(const QString& file);
    
    // Completion
    void setCompleted(bool success, const QString& message = "");
    
    // Options
    bool getAutoInstall() const;
    bool getCreateBackup() const;

signals:
    void installRequested();
    void laterRequested();
    void skipRequested();

private slots:
    void onInstallClicked();
    void onLaterClicked();
    void onSkipClicked();

private:
    QLabel* version_label_{nullptr};
    QLabel* status_label_{nullptr};
    QTextEdit* changelog_edit_{nullptr};
    QProgressBar* progress_bar_{nullptr};
    QLabel* current_file_label_{nullptr};
    QCheckBox* auto_install_check_{nullptr};
    QCheckBox* create_backup_check_{nullptr};
    QPushButton* install_button_{nullptr};
    QPushButton* later_button_{nullptr};
    QPushButton* skip_button_{nullptr};
    
    bool mandatory_{false};
    
    void setupUI();
};

}  // namespace ui
}  // namespace cad
