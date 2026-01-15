#include "QtCommandLine.h"

namespace cad {
namespace ui {

QtCommandLine::QtCommandLine(QWidget* parent) : QLineEdit(parent) {
    setPlaceholderText(tr("Command"));
}

}  // namespace ui
}  // namespace cad
