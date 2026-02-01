#include "SurfaceService.h"

namespace cad {
namespace modules {

bool SurfaceService::patch(const std::string&) { return false; }
bool SurfaceService::trim(const std::string&, const std::string&) { return false; }
bool SurfaceService::stitch(const std::vector<std::string>&) { return false; }
bool SurfaceService::thicken(const std::string&, double) { return false; }

}  // namespace modules
}  // namespace cad
