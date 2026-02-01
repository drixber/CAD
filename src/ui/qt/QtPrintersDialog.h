#pragma once

#include <QDialog>
#include <memory>
#include <string>
#include <vector>

struct PrinterProfile;

namespace cad {
namespace app {
class PrinterService;
}
}

class QListWidget;
class QLineEdit;
class QComboBox;
class QPushButton;
class QLabel;

namespace cad {
namespace ui {

class QtPrintersDialog : public QDialog {
    Q_OBJECT
public:
    explicit QtPrintersDialog(QWidget* parent = nullptr);
    void setPrinterService(cad::app::PrinterService* service);
    /** When true, only list and "Send to printer" button are shown (e.g. for Send-to-Print dialog). */
    void setSendMode(bool send_only);

    /** Returns selected printer id for "Send to printer" flow; empty if none selected. */
    std::string selectedPrinterId() const;

private slots:
    void onPrinterSelected();
    void onAddClicked();
    void onRemoveClicked();
    void onTestClicked();
    void onSaveClicked();

private:
    void refreshList();
    void loadToForm(const std::string& id);
    void clearForm();

    cad::app::PrinterService* printer_service_{nullptr};
    QListWidget* list_{nullptr};
    QLineEdit* name_edit_{nullptr};
    QComboBox* type_combo_{nullptr};
    QLineEdit* url_edit_{nullptr};
    QLineEdit* api_key_edit_{nullptr};
    QPushButton* remove_btn_{nullptr};
    QPushButton* test_btn_{nullptr};
    QPushButton* save_btn_{nullptr};
    QLabel* status_label_{nullptr};
    QPushButton* send_to_btn_{nullptr};
    QWidget* form_container_{nullptr};
    bool send_only_mode_{false};
    std::string current_id_;
};

}  // namespace ui
}  // namespace cad
