#include "AnnotationService.h"

namespace cad {
namespace drawings {

std::vector<Annotation> AnnotationService::buildDefaultAnnotations(const std::string& sheet) const {
    std::vector<Annotation> annotations;
    if (!sheet.empty()) {
        annotations.push_back({"Sheet note", 10.0, 10.0});
        annotations.push_back({"Revision A", 10.0, 20.0});
        annotations.push_back({"Scale 1:1", 10.0, 30.0});
        annotations.push_back({"Parts list", 10.0, 40.0});
    }
    return annotations;
}

std::vector<Dimension> AnnotationService::buildDefaultDimensions(const std::string& sheet) const {
    std::vector<Dimension> dimensions;
    if (!sheet.empty()) {
        dimensions.push_back({"Width", 100.0, "mm", "+/-0.1"});
        dimensions.push_back({"Height", 50.0, "mm", "+/-0.1"});
        dimensions.push_back({"Depth", 25.0, "mm", "+/-0.1"});
    }
    return dimensions;
}

}  // namespace drawings
}  // namespace cad
