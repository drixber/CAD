#pragma once

#include <string>

namespace cad {
namespace ui {

class CommandLine {
public:
    void setPrompt(const std::string& prompt);
    const std::string& prompt() const;

private:
    std::string prompt_;
};

}  // namespace ui
}  // namespace cad
