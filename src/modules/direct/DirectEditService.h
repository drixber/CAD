#pragma once

#include <string>

namespace cad {
namespace modules {

enum class DirectEditOperation {
    MoveFace,
    OffsetFace,
    DeleteFace,
    Freeform
};

struct DirectEditRequest {
    std::string targetFeature;
    DirectEditOperation operation;
};

struct DirectEditResult {
    bool success{false};
    std::string message;
};

class DirectEditService {
public:
    DirectEditResult applyEdit(const DirectEditRequest& request) const;
};

}  // namespace modules
}  // namespace cad
