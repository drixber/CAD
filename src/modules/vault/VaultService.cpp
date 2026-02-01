#include "VaultService.h"

namespace cad {
namespace modules {

bool VaultService::checkOut(const std::string&) { return false; }
bool VaultService::checkIn(const std::string&, const std::string&) { return false; }
std::vector<VaultService::FileVersion> VaultService::getVersionHistory(const std::string&) const { return {}; }

}  // namespace modules
}  // namespace cad
