#include "AnnotationService.h"

namespace cad {
namespace drawings {

std::vector<Annotation> AnnotationService::buildDefaultAnnotations(const std::string& sheet) const {
    std::vector<Annotation> annotations;
    if (!sheet.empty()) {
        annotations.push_back({"Sheet note", 10.0, 10.0});
        annotations.push_back({"Revision A", 10.0, 20.0});
    }
    return annotations;
}

std::vector<Dimension> AnnotationService::buildDefaultDimensions(const std::string& sheet) const {
    std::vector<Dimension> dimensions;
    if (!sheet.empty()) {
        dimensions.push_back({"Width", 100.0, "mm"});
        dimensions.push_back({"Height", 50.0, "mm"});
    }
    return dimensions;
}

}  // namespace drawings
}  // namespace cad
