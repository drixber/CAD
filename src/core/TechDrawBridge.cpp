#include "TechDrawBridge.h"

namespace cad {
namespace core {

bool TechDrawBridge::initialize() { return false; }
bool TechDrawBridge::syncDrawing(const cad::drawings::DrawingDocument&) { return false; }
bool TechDrawBridge::syncAssociativeLinks(const cad::drawings::DrawingDocument&) { return false; }
bool TechDrawBridge::syncDimensions(const cad::drawings::DrawingDocument&) { return false; }
bool TechDrawBridge::createBaseView(const std::string&, const std::string&, const std::string&, double) { return false; }
bool TechDrawBridge::createSectionView(const std::string&, const std::string&, const std::string&, double) { return false; }
bool TechDrawBridge::createDetailView(const std::string&, const std::string&, double, double, double, double) { return false; }
bool TechDrawBridge::addDimension(const std::string&, const std::string&, double, double, double, double, double) { return false; }
bool TechDrawBridge::addBomTable(const std::string&, const std::vector<std::tuple<std::string, int, std::string>>&) { return false; }
bool TechDrawBridge::applySheetTemplate(const std::string&, const std::string&) { return false; }
std::vector<double> TechDrawBridge::parseOrientation(const std::string&) const { return {}; }
std::vector<double> TechDrawBridge::parseCutPlane(const std::string&) const { return {}; }

}  // namespace core
}  // namespace cad
