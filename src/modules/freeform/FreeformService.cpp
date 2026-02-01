#include "FreeformService.h"

namespace cad {
namespace modules {

std::string FreeformService::createFromPrimitive(const std::string&) { return {}; }
bool FreeformService::pull(const std::string&, int, double, double, double) { return false; }
bool FreeformService::smooth(const std::string&) { return false; }
bool FreeformService::thickenToSolid(const std::string&, double) { return false; }

}  // namespace modules
}  // namespace cad
