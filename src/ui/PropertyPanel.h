#pragma once

#include <string>

namespace cad {
namespace ui {

class PropertyPanel {
public:
    void setActiveContext(const std::string& context);
    const std::string& activeContext() const;

private:
    std::string active_context_;
};

}  // namespace ui
}  // namespace cad
