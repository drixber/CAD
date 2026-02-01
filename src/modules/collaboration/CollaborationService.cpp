#include "CollaborationService.h"

namespace cad {
namespace modules {

bool CollaborationService::shareProject(const std::string&, const std::string&, const std::string&) { return false; }
bool CollaborationService::addComment(const std::string&, const Comment&) { return false; }
std::vector<CollaborationService::Comment> CollaborationService::getComments(const std::string&) const { return {}; }
bool CollaborationService::setUserRole(const std::string&, const std::string&, const std::string&) { return false; }

}  // namespace modules
}  // namespace cad
