#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>

namespace cad {
namespace ui {

class QtLoginDialog : public QDialog {
    Q_OBJECT

public:
    explicit QtLoginDialog(QWidget* parent = nullptr);
    ~QtLoginDialog() override = default;
    
    QString getUsername() const;
    QString getPassword() const;
    bool getRememberMe() const;
    bool shouldShowRegister() const;
    
    void setError(const QString& error_message);
    void setRememberedUsername(const QString& username);

signals:
    void loginRequested(const QString& username, const QString& password, bool remember_me);
    void registerRequested();

private slots:
    void onLoginClicked();
    void onRegisterClicked();
    void onTextChanged();

private:
    QLineEdit* username_edit_{nullptr};
    QLineEdit* password_edit_{nullptr};
    QPushButton* login_button_{nullptr};
    QPushButton* register_button_{nullptr};
    QCheckBox* remember_me_checkbox_{nullptr};
    QLabel* error_label_{nullptr};
};

}  // namespace ui
}  // namespace cad
