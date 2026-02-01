#include "ContentCenterService.h"

namespace cad {
namespace modules {

std::vector<ContentCenterService::ContentItem> ContentCenterService::search(const std::string& family,
                                                                           const std::string& keyword) const {
    (void)family;
    (void)keyword;
    return {};
}

bool ContentCenterService::placePart(const std::string& item_id, const std::string& assembly_id) const {
    (void)item_id;
    (void)assembly_id;
    return false;
}

}  // namespace modules
}  // namespace cad
