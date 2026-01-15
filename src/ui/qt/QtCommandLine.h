#pragma once

#include <QLineEdit>

namespace cad {
namespace ui {

class QtCommandLine : public QLineEdit {
    Q_OBJECT

public:
    explicit QtCommandLine(QWidget* parent = nullptr);
};

}  // namespace ui
}  // namespace cad
