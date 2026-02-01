#pragma once

#include <string>
#include <vector>

namespace cad {
namespace core {
class Part;
struct Transform;
}
namespace modules {

/** User Defined Feature (Creo/darius): wiederverwendbare Feature-Gruppe, Bibliothek + Einfügen. */
class UdfService {
public:
    /** Feature-Gruppe als UDF speichern (Bibliothek). */
    bool saveUdf(const cad::core::Part& part, const std::vector<std::string>& feature_names,
                 const std::string& file_path) const;
    /** UDF in Teil einfügen (mit Transform). */
    bool insertUdf(cad::core::Part& part, const std::string& file_path,
                   const cad::core::Transform& transform) const;
    /** Liste der verfügbaren UDFs in einem Verzeichnis. */
    std::vector<std::string> listUdfs(const std::string& directory_path) const;
};

}  // namespace modules
}  // namespace cad
