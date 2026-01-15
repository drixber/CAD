#pragma once

#include <string>

namespace cad {
namespace core {

class Command {
public:
    virtual ~Command() = default;

    virtual void execute() = 0;
    virtual void undo() = 0;
    virtual std::string name() const = 0;
};

}  // namespace core
}  // namespace cad
