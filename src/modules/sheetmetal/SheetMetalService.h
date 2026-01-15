#pragma once

#include <string>

namespace cad {
namespace modules {

enum class SheetMetalOperation {
    Flange,
    Face,
    Cut,
    Bend,
    Unfold,
    Refold
};

struct SheetMetalRequest {
    std::string targetPart;
    SheetMetalOperation operation;
};

struct SheetMetalResult {
    bool success{false};
    std::string message;
};

class SheetMetalService {
public:
    SheetMetalResult applyOperation(const SheetMetalRequest& request) const;
};

}  // namespace modules
}  // namespace cad
