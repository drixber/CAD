#include "UndoStack.h"

namespace cad {
namespace core {

void UndoStack::execute(std::unique_ptr<Command> command) {
    if (!command) {
        return;
    }
    if (index_ < commands_.size()) {
        commands_.erase(commands_.begin() + static_cast<long>(index_), commands_.end());
    }
    command->execute();
    commands_.push_back(std::move(command));
    index_ = commands_.size();
}

bool UndoStack::canUndo() const {
    return index_ > 0 && !commands_.empty();
}

bool UndoStack::canRedo() const {
    return index_ < commands_.size();
}

std::string UndoStack::undo() {
    if (!canUndo()) {
        return {};
    }
    --index_;
    commands_[index_]->undo();
    return commands_[index_]->name();
}

std::string UndoStack::redo() {
    if (!canRedo()) {
        return {};
    }
    commands_[index_]->execute();
    ++index_;
    return commands_[index_ - 1]->name();
}

void UndoStack::clear() {
    commands_.clear();
    index_ = 0;
}

std::size_t UndoStack::getHistorySize() const {
    return commands_.size();
}

std::size_t UndoStack::getUndoCount() const {
    return index_;
}

std::size_t UndoStack::getRedoCount() const {
    return commands_.size() - index_;
}

std::vector<std::string> UndoStack::getCommandHistory() const {
    std::vector<std::string> history;
    history.reserve(commands_.size());
    for (const auto& cmd : commands_) {
        history.push_back(cmd->name());
    }
    return history;
}

void UndoStack::setMaxHistorySize(std::size_t max_size) {
    max_history_size_ = max_size;
    // Trim history if needed
    if (commands_.size() > max_history_size_) {
        std::size_t to_remove = commands_.size() - max_history_size_;
        commands_.erase(commands_.begin(), commands_.begin() + static_cast<long>(to_remove));
        if (index_ > to_remove) {
            index_ -= to_remove;
        } else {
            index_ = 0;
        }
    }
}

}  // namespace core
}  // namespace cad
