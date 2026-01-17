#pragma once

#include <string>
#include <vector>

#include "model/DrawingDocument.h"

namespace cad {
namespace drawings {

class BomService {
public:
    std::vector<BillOfMaterialsItem> buildBom(const std::string& assembly_id) const;
};

}  // namespace drawings
}  // namespace cad
