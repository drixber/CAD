#include "UdfService.h"
#include "core/Modeler/Part.h"
#include "core/Modeler/Transform.h"
#include <filesystem>

namespace cad {
namespace modules {

bool UdfService::saveUdf(const cad::core::Part&, const std::vector<std::string>&,
                         const std::string&) const {
    return false;
}

bool UdfService::insertUdf(cad::core::Part&, const std::string&,
                            const cad::core::Transform&) const {
    return false;
}

std::vector<std::string> UdfService::listUdfs(const std::string& directory_path) const {
    std::vector<std::string> out;
    try {
        for (const auto& e : std::filesystem::directory_iterator(directory_path)) {
            if (e.path().extension() == ".udf")
                out.push_back(e.path().filename().string());
        }
    } catch (...) {}
    return out;
}

}  // namespace modules
}  // namespace cad
