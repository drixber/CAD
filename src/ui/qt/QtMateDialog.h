#pragma once

#include <QDialog>
#include <QString>

class QComboBox;
class QDoubleSpinBox;
class QCheckBox;

namespace cad {
namespace ui {

/** Dialog zur Auswahl des Mate-Typs und optionalen Werts (Abstand/Winkel). */
class QtMateDialog : public QDialog {
    Q_OBJECT
public:
    explicit QtMateDialog(QWidget* parent = nullptr);

    QString mateType() const;
    double mateValue() const;
    bool isValueUsed() const;

private:
    void onTypeChanged(const QString& type);

    QComboBox* type_combo_{nullptr};
    QDoubleSpinBox* value_spin_{nullptr};
    QCheckBox* value_check_{nullptr};
};

}  // namespace ui
}  // namespace cad
