#include "BomService.h"

#include <algorithm>
#include <map>
#include <sstream>

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

std::vector<BillOfMaterialsItem> BomService::buildBomFromAssembly(const cad::core::Assembly& assembly) const {
    std::map<std::string, int> part_counts;
    std::map<std::string, std::string> part_numbers;
    
    // Count parts and track first occurrence for part number generation
    int part_index = 1;
    for (const auto& component : assembly.components()) {
        const std::string& part_name = component.part.name();
        part_counts[part_name]++;
        if (part_numbers.find(part_name) == part_numbers.end()) {
            part_numbers[part_name] = generatePartNumber(part_name, part_index++);
        }
    }
    
    // Build BOM items from counts
    std::vector<BillOfMaterialsItem> items;
    for (const auto& [part_name, quantity] : part_counts) {
        BillOfMaterialsItem item;
        item.part_name = part_name;
        item.quantity = quantity;
        item.part_number = part_numbers[part_name];
        items.push_back(item);
    }
    
    // Sort by part name for consistent BOM ordering
    std::sort(items.begin(), items.end(),
              [](const BillOfMaterialsItem& a, const BillOfMaterialsItem& b) {
                  return a.part_name < b.part_name;
              });
    
    return items;
}

std::string BomService::generatePartNumber(const std::string& part_name, int index) const {
    if (part_name.empty()) {
        std::ostringstream oss;
        oss << "PNT-" << index;
        return oss.str();
    }
    
    // Generate part number from part name: take first 3 uppercase letters + index
    std::ostringstream oss;
    int letters_found = 0;
    for (char c : part_name) {
        if (std::isalpha(c) && letters_found < 3) {
            oss << static_cast<char>(std::toupper(c));
            letters_found++;
        }
    }
    
    // If we didn't get 3 letters, pad with numbers
    while (letters_found < 3) {
        oss << static_cast<char>('0' + (index % 10));
        letters_found++;
    }
    
    oss << "-" << (index < 10 ? "00" : index < 100 ? "0" : "") << index;
    return oss.str();
}

}  // namespace drawings
}  // namespace cad
