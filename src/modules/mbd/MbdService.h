#pragma once

#include <string>

namespace cad {
namespace modules {

struct MbdRequest {
    std::string targetPart;
    std::string note;
};

struct MbdResult {
    bool success{false};
    std::string message;
};

class MbdService {
public:
    MbdResult applyMbd(const MbdRequest& request) const;
};

}  // namespace modules
}  // namespace cad
