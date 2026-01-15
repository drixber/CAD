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

}  // namespace core
}  // namespace cad
