#include "UndoStack.h"

#include <map>
#include <algorithm>

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
    trimHistory();
}

void UndoStack::createSnapshot(const std::string& snapshot_name) {
    StateSnapshot snapshot;
    snapshot.name = snapshot_name;
    snapshot.index = index_;
    
    serializeState(snapshot);
    snapshots_[snapshot_name] = std::move(snapshot);
}

bool UndoStack::restoreSnapshot(const std::string& snapshot_name) {
    auto it = snapshots_.find(snapshot_name);
    if (it == snapshots_.end()) {
        return false;
    }
    
    deserializeState(it->second);
    return true;
}

std::vector<std::string> UndoStack::getSnapshotNames() const {
    std::vector<std::string> names;
    names.reserve(snapshots_.size());
    for (const auto& pair : snapshots_) {
        names.push_back(pair.first);
    }
    return names;
}

void UndoStack::clearSnapshots() {
    snapshots_.clear();
}

void UndoStack::beginGroup(const std::string& group_name) {
    if (is_grouping_) {
        endGroup();
    }
    current_group_name_ = group_name;
    is_grouping_ = true;
    grouped_commands_.clear();
}

void UndoStack::endGroup() {
    if (!is_grouping_ || grouped_commands_.empty()) {
        is_grouping_ = false;
        return;
    }
    
    // Create a grouped command that executes all commands in the group
    // In real implementation, this would create a GroupCommand
    is_grouping_ = false;
    grouped_commands_.clear();
}

bool UndoStack::isGrouping() const {
    return is_grouping_;
}

void UndoStack::trimHistory() {
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

void UndoStack::serializeState(StateSnapshot& snapshot) const {
    snapshot.commands.clear();
    snapshot.index = index_;
    
    for (std::size_t i = 0; i < index_ && i < commands_.size(); ++i) {
        // In real implementation, commands would be serialized
        // For now, we just store the command names
        snapshot.commands.push_back(nullptr);
    }
}

void UndoStack::deserializeState(const StateSnapshot& snapshot) {
    index_ = snapshot.index;
    // In real implementation, commands would be deserialized
    // For now, we just restore the index
}

}  // namespace core
}  // namespace cad
