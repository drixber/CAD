#include "GenerativeService.h"

namespace cad {
namespace modules {

std::string GenerativeService::createStudy(const DesignSpace&, const std::string&) { return {}; }
std::vector<std::string> GenerativeService::getResultVariants(const std::string&) const { return {}; }

}  // namespace modules
}  // namespace cad
