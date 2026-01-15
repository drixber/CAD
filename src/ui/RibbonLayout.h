#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace cad {
namespace ui {

class RibbonLayout {
public:
    void configureDefaultTabs();
    const std::vector<std::string>& tabs() const;
    const std::unordered_map<std::string, std::vector<std::string>>& commands() const;

private:
    std::vector<std::string> tabs_;
    std::unordered_map<std::string, std::vector<std::string>> commands_;
};

}  // namespace ui
}  // namespace cad
