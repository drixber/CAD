#include "PropertyPanel.h"

namespace cad {
namespace ui {

void PropertyPanel::setActiveContext(const std::string& context) {
    active_context_ = context;
}

const std::string& PropertyPanel::activeContext() const {
    return active_context_;
}

}  // namespace ui
}  // namespace cad
