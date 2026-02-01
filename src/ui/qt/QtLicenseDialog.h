#pragma once

#include <QDialog>
#include <functional>
#include <string>

class QLineEdit;
class QLabel;
class QPushButton;

namespace cad {
namespace ui {

class QtLicenseDialog : public QDialog {
    Q_OBJECT
public:
    explicit QtLicenseDialog(QWidget* parent = nullptr);
    void setActivateCallback(std::function<bool(const std::string& key)> callback);

private:
    void onActivateClicked();
    QLineEdit* key_edit_{nullptr};
    QLabel* status_label_{nullptr};
    QPushButton* activate_btn_{nullptr};
    std::function<bool(const std::string&)> activate_callback_;
};

}  // namespace ui
}  // namespace cad
