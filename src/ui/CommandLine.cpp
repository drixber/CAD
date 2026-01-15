#include "CommandLine.h"

namespace cad {
namespace ui {

void CommandLine::setPrompt(const std::string& prompt) {
    prompt_ = prompt;
}

const std::string& CommandLine::prompt() const {
    return prompt_;
}

}  // namespace ui
}  // namespace cad
