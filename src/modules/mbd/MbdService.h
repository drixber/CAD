#pragma once

#include <string>

#include "PmiData.h"

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
    cad::mbd::PmiDataSet buildDefaultPmi(const std::string& part_id) const;
};

}  // namespace modules
}  // namespace cad
