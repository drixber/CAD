#include <gtest/gtest.h>
#include "core/undo/UndoStack.h"
#include "core/undo/Command.h"

using namespace cad::core;

class TestCommand : public Command {
public:
    TestCommand(const std::string& name) : name_(name), executed_(false) {}
    
    void execute() override { executed_ = true; }
    void undo() override { executed_ = false; }
    std::string name() const override { return name_; }
    
    bool isExecuted() const { return executed_; }
    
private:
    std::string name_;
    bool executed_;
};

TEST(UndoStackExtendedTest, StateSnapshots) {
    UndoStack stack;
    
    stack.execute(std::make_unique<TestCommand>("cmd1"));
    stack.execute(std::make_unique<TestCommand>("cmd2"));
    
    stack.createSnapshot("snapshot1");
    
    stack.execute(std::make_unique<TestCommand>("cmd3"));
    
    bool restored = stack.restoreSnapshot("snapshot1");
    ASSERT_TRUE(restored);
    
    std::vector<std::string> snapshots = stack.getSnapshotNames();
    ASSERT_FALSE(snapshots.empty());
}

TEST(UndoStackExtendedTest, CommandGrouping) {
    UndoStack stack;
    
    stack.beginGroup("group1");
    stack.execute(std::make_unique<TestCommand>("cmd1"));
    stack.execute(std::make_unique<TestCommand>("cmd2"));
    stack.endGroup();
    
    ASSERT_FALSE(stack.isGrouping());
    ASSERT_EQ(stack.getHistorySize(), 1);  // Grouped commands count as one
}

TEST(UndoStackExtendedTest, MemoryEfficient) {
    UndoStack stack;
    stack.setMaxHistorySize(10);
    
    for (int i = 0; i < 20; ++i) {
        stack.execute(std::make_unique<TestCommand>("cmd" + std::to_string(i)));
    }
    
    ASSERT_LE(stack.getHistorySize(), 10);
}
