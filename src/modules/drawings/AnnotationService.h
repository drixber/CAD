#pragma once

#include <string>
#include <vector>

#include "model/DrawingDocument.h"

namespace cad {
namespace drawings {

class AnnotationService {
public:
    std::vector<Annotation> buildDefaultAnnotations(const std::string& sheet) const;
    std::vector<Dimension> buildDefaultDimensions(const std::string& sheet) const;
};

}  // namespace drawings
}  // namespace cad
