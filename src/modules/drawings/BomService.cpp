#include "BomService.h"

namespace cad {
namespace drawings {

std::vector<BillOfMaterialsItem> BomService::buildBom(const std::string& assembly_id) const {
    std::vector<BillOfMaterialsItem> items;
    if (!assembly_id.empty()) {
        items.push_back({"Bracket", 1, "BRK-001"});
        items.push_back({"Plate", 1, "PLT-001"});
    }
    return items;
}

}  // namespace drawings
}  // namespace cad
