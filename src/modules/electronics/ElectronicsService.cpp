#include "ElectronicsService.h"

namespace cad {
namespace modules {

std::string ElectronicsService::createSchematic(const std::string&) { return {}; }
std::string ElectronicsService::createPcb(const std::string&) { return {}; }
bool ElectronicsService::routePcb(const std::string&) { return false; }
std::string ElectronicsService::exportTo3D(const std::string&) { return {}; }

}  // namespace modules
}  // namespace cad
