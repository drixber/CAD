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
    
    // History management
    std::size_t getHistorySize() const;
    std::size_t getUndoCount() const;
    std::size_t getRedoCount() const;
    std::vector<std::string> getCommandHistory() const;
    void setMaxHistorySize(std::size_t max_size);

private:
    std::vector<std::unique_ptr<Command>> commands_{};
    std::size_t index_{0};
    std::size_t max_history_size_{100};  // Default: keep last 100 commands
};

}  // namespace core
}  // namespace cad
