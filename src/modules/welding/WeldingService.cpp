#include "WeldingService.h"

#include <algorithm>
#include <cmath>
#include <map>

namespace cad {
namespace modules {

bool WeldingService::addWeld(const std::string& assembly_id, const WeldJoint& weld) {
    WeldJoint w = weld;
    if (w.name.empty()) {
        w.name = "Weld_" + std::to_string(assembly_welds_[assembly_id].size() + 1);
    }
    assembly_welds_[assembly_id].push_back(w);
    return true;
}

std::vector<WeldJoint> WeldingService::getWelds(const std::string& assembly_id) const {
    auto it = assembly_welds_.find(assembly_id);
    if (it != assembly_welds_.end()) {
        return it->second;
    }
    return {};
}

std::vector<WeldBomItem> WeldingService::getWeldBom(const std::string& assembly_id) const {
    std::vector<WeldBomItem> bom;
    std::map<std::string, WeldBomItem> by_type;
    for (const auto& w : getWelds(assembly_id)) {
        std::string key;
        switch (w.type) {
            case WeldType::Fillet: key = "Fillet"; break;
            case WeldType::Butt:   key = "Butt"; break;
            case WeldType::V:     key = "V"; break;
            case WeldType::U:     key = "U"; break;
            case WeldType::Spot:  key = "Spot"; break;
            case WeldType::Cosmetic: key = "Cosmetic"; break;
            default: key = "Fillet"; break;
        }
        key += "_" + std::to_string(static_cast<int>(w.size_mm));
        if (by_type.find(key) == by_type.end()) {
            WeldBomItem item;
            item.weld_type_name = (key.substr(0, key.find('_')));
            item.symbol = getSymbolText(w);
            item.quantity = 0;
            item.total_length_mm = 0.0;
            by_type[key] = item;
        }
        by_type[key].quantity++;
        by_type[key].total_length_mm += w.length_mm > 0.0 ? w.length_mm : 10.0;
    }
    for (const auto& kv : by_type) {
        bom.push_back(kv.second);
    }
    return bom;
}

double WeldingService::estimateWeldVolume(const WeldJoint& weld) const {
    double length = weld.length_mm > 0.0 ? weld.length_mm : 10.0;
    switch (weld.type) {
        case WeldType::Fillet:
            return 0.5 * weld.size_mm * weld.size_mm * length;
        case WeldType::Butt:
        case WeldType::V:
        case WeldType::U:
            return weld.size_mm * weld.throat_mm * length;
        case WeldType::Spot:
            return 4.0 / 3.0 * 3.14159 * std::pow(weld.size_mm * 0.5, 3);
        default:
            return 0.0;
    }
}

std::string WeldingService::getSymbolText(const WeldJoint& weld) const {
    std::string type_str;
    switch (weld.type) {
        case WeldType::Fillet: type_str = "Fillet"; break;
        case WeldType::Butt:   type_str = "Butt"; break;
        case WeldType::V:     type_str = "V"; break;
        case WeldType::U:     type_str = "U"; break;
        case WeldType::Spot:  type_str = "Spot"; break;
        case WeldType::Cosmetic: type_str = "Cosmetic"; break;
        default: type_str = "Fillet"; break;
    }
    if (weld.size_mm > 0.0) {
        type_str += " " + std::to_string(static_cast<int>(weld.size_mm));
    }
    return type_str;
}

}  // namespace modules
}  // namespace cad
