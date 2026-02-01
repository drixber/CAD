#include "ReuseLibraryService.h"

namespace cad {
namespace modules {

std::vector<ReuseLibraryService::ReuseItem> ReuseLibraryService::listItems(const std::string& category) const {
    (void)category;
    return {};
}

bool ReuseLibraryService::addToPart(const std::string& item_id, const std::string& part_id) const {
    (void)item_id;
    (void)part_id;
    return false;
}

}  // namespace modules
}  // namespace cad
