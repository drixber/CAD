#include "MbdService.h"

namespace cad {
namespace modules {

MbdResult MbdService::applyMbd(const MbdRequest& request) const {
    MbdResult result;
    if (!request.targetPart.empty()) {
        result.success = true;
        result.message = "MBD applied";
    }
    return result;
}

cad::mbd::PmiDataSet MbdService::buildDefaultPmi(const std::string& part_id) const {
    cad::mbd::PmiDataSet data;
    data.part_id = part_id;
    data.datums.push_back({"A", "Plane", "Primary datum"});
    data.tolerances.push_back({"Profile", 0.1, -0.1, "mm"});
    data.annotations.push_back({"PMI note", 0.0, 0.0, 0.0});
    return data;
}

}  // namespace modules
}  // namespace cad
