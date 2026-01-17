#pragma once

#include <string>

#include "model/DrawingDocument.h"

namespace cad {
namespace drawings {

class AssociativeLinkService {
public:
    bool updateFromModel(DrawingDocument& document, const std::string& model_id) const;
};

}  // namespace drawings
}  // namespace cad
