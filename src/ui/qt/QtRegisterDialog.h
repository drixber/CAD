#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

namespace cad {
namespace ui {

class QtRegisterDialog : public QDialog {
    Q_OBJECT

public:
    explicit QtRegisterDialog(QWidget* parent = nullptr);
    ~QtRegisterDialog() override = default;
    
    QString getUsername() const;
    QString getEmail() const;
    QString getPassword() const;
    QString getConfirmPassword() const;
    
    void setError(const QString& error_message);

signals:
    void registerRequested(const QString& username, const QString& email, 
                          const QString& password, const QString& confirm_password);

private slots:
    void onRegisterClicked();
    void onTextChanged();

private:
    QLineEdit* username_edit_{nullptr};
    QLineEdit* email_edit_{nullptr};
    QLineEdit* password_edit_{nullptr};
    QLineEdit* confirm_password_edit_{nullptr};
    QPushButton* register_button_{nullptr};
    QLabel* error_label_{nullptr};
};

}  // namespace ui
}  // namespace cad
