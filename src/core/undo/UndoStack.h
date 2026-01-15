#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Command.h"

namespace cad {
namespace core {

class UndoStack {
public:
    void execute(std::unique_ptr<Command> command);
    bool canUndo() const;
    bool canRedo() const;
    std::string undo();
    std::string redo();
    void clear();

private:
    std::vector<std::unique_ptr<Command>> commands_{};
    std::size_t index_{0};
};

}  // namespace core
}  // namespace cad
