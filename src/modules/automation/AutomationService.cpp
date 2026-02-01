#include "AutomationService.h"

namespace cad {
namespace modules {

bool AutomationService::runScript(const std::string&, const std::string&) { return false; }
bool AutomationService::recordMacro() { return false; }
bool AutomationService::stopMacro() { return false; }
std::string AutomationService::getRecordedMacro() const { return {}; }

}  // namespace modules
}  // namespace cad
