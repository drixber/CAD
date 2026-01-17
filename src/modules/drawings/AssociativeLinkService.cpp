#include "AssociativeLinkService.h"

namespace cad {
namespace drawings {

bool AssociativeLinkService::updateFromModel(DrawingDocument& document,
                                             const std::string& model_id) const {
    if (document.source_model_id != model_id) {
        return false;
    }
    document.revision += 1;
    document.last_update_id = model_id;
    return true;
}

}  // namespace drawings
}  // namespace cad
