#pragma once

#include "modules/drawings/model/DrawingDocument.h"
#include <vector>
#include <string>

namespace cad {
namespace core {

class TechDrawBridge {
public:
    bool initialize();
    bool syncDrawing(const cad::drawings::DrawingDocument& document);
    bool syncAssociativeLinks(const cad::drawings::DrawingDocument& document);
    bool syncDimensions(const cad::drawings::DrawingDocument& document);

    bool createBaseView(const std::string& view_name, const std::string& source_id, const std::string& orientation, double scale);
    bool createSectionView(const std::string& view_name, const std::string& source_id, const std::string& cut_plane, double scale);
    bool createDetailView(const std::string& view_name, const std::string& source_id, double center_x, double center_y, double radius, double scale);
    bool addDimension(const std::string& view_name, const std::string& dim_label, double value, double x1, double y1, double x2, double y2);
    bool addBomTable(const std::string& sheet_name, const std::vector<std::tuple<std::string, int, std::string>>& bom_items);
    bool applySheetTemplate(const std::string& sheet_name, const std::string& template_name);

private:
    std::vector<double> parseOrientation(const std::string& orientation) const;
    std::vector<double> parseCutPlane(const std::string& cut_plane) const;
};

}  // namespace core
}  // namespace cad
