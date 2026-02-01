#include "CamService.h"

namespace cad {
namespace modules {

bool CamService::addTool(const ToolSpec&) { return false; }
std::string CamService::generateToolpath(const std::string&, const std::string&) { return {}; }
bool CamService::exportGCode(const std::string&, const std::string&) { return false; }

}  // namespace modules
}  // namespace cad
