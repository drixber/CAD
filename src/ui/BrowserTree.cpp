#include "BrowserTree.h"

namespace cad {
namespace ui {

void BrowserTree::setRootLabel(const std::string& label) {
    root_label_ = label;
}

const std::string& BrowserTree::rootLabel() const {
    return root_label_;
}

}  // namespace ui
}  // namespace cad
