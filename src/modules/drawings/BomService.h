#pragma once

#include <string>
#include <vector>

#include "../../core/Modeler/Assembly.h"
#include "model/DrawingDocument.h"

namespace cad {
namespace drawings {

class BomService {
public:
    std::vector<BillOfMaterialsItem> buildBom(const std::string& assembly_id) const;
    std::vector<BillOfMaterialsItem> buildBomFromAssembly(const cad::core::Assembly& assembly) const;

private:
    std::string generatePartNumber(const std::string& part_name, int index) const;
};

}  // namespace drawings
}  // namespace cad
