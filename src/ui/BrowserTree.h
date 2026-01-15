#pragma once

#include <string>

namespace cad {
namespace ui {

class BrowserTree {
public:
    void setRootLabel(const std::string& label);
    const std::string& rootLabel() const;

private:
    std::string root_label_;
};

}  // namespace ui
}  // namespace cad
