#pragma once

#include <memory>
#include <string>
#include <vector>
#include <map>

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
    
    // State snapshots
    void createSnapshot(const std::string& snapshot_name);
    bool restoreSnapshot(const std::string& snapshot_name);
    std::vector<std::string> getSnapshotNames() const;
    void clearSnapshots();
    
    // Command grouping
    void beginGroup(const std::string& group_name);
    void endGroup();
    bool isGrouping() const;

private:
    struct StateSnapshot {
        std::string name;
        std::vector<std::unique_ptr<Command>> commands;
        std::size_t index;
    };
    
    std::vector<std::unique_ptr<Command>> commands_{};
    std::size_t index_{0};
    std::size_t max_history_size_{100};  // Default: keep last 100 commands
    
    std::map<std::string, StateSnapshot> snapshots_;
    std::string current_group_name_;
    std::vector<std::unique_ptr<Command>> grouped_commands_;
    bool is_grouping_{false};
    
    void trimHistory();
    void serializeState(StateSnapshot& snapshot) const;
    void deserializeState(const StateSnapshot& snapshot);
};

}  // namespace core
}  // namespace cad
