#include "CompositesService.h"

namespace cad {
namespace modules {

std::string CompositesService::createCompositePart(const std::string&) { return {}; }
bool CompositesService::addPly(const std::string&, const Ply&) { return false; }
std::vector<CompositesService::Ply> CompositesService::getPlies(const std::string&) const { return {}; }
bool CompositesService::exportLayupReport(const std::string&, const std::string&) const { return false; }

}  // namespace modules
}  // namespace cad
