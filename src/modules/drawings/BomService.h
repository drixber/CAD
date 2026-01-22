#pragma once

#include <map>
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
    
    // Registry methods for UI integration
    void registerAssembly(const std::string& assembly_id, const cad::core::Assembly& assembly);
    bool hasAssembly(const std::string& assembly_id) const;
    std::vector<BillOfMaterialsItem> getBomForAssembly(const std::string& assembly_id) const;
    std::vector<std::string> getRegisteredAssemblyIds() const;
    void unregisterAssembly(const std::string& assembly_id);
    void clearRegistry();

private:
    std::string generatePartNumber(const std::string& part_name, int index) const;
    std::map<std::string, cad::core::Assembly> assembly_registry_;
};

}  // namespace drawings
}  // namespace cad
