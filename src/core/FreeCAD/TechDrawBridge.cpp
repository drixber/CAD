#include "TechDrawBridge.h"

#include <cmath>

#ifdef CAD_USE_FREECAD
#include <App/Application.h>
#include <App/Document.h>
#include <App/DocumentObject.h>
#include <vector>
#include <map>
#include <string>
#endif

namespace cad {
namespace core {

bool TechDrawBridge::initialize() {
    return true;
}

bool TechDrawBridge::syncDrawing(const cad::drawings::DrawingDocument& document) {
#ifdef CAD_USE_FREECAD
    App::Application* app = App::GetApplication();
    if (!app) {
        return false;
    }
    App::Document* doc = app->getActiveDocument();
    if (!doc) {
        return false;
    }
    
    App::DocumentObject* page = doc->addObject("TechDraw::DrawPage", document.title().c_str());
    if (!page) {
        return false;
    }
    
    std::map<std::string, double> page_params;
    page_params["Width"] = 210.0;
    page_params["Height"] = 297.0;
    page_params["Scale"] = 1.0;
    page->setPropertyByName("Width", &page_params["Width"]);
    page->setPropertyByName("Height", &page_params["Height"]);
    page->setPropertyByName("Scale", &page_params["Scale"]);
    
    for (const auto& sheet : document.sheets()) {
        for (const auto& view : sheet.views()) {
            App::DocumentObject* view_obj = doc->addObject("TechDraw::DrawViewPart", view.name().c_str());
            if (!view_obj) {
                continue;
            }
            
            std::map<std::string, double> view_params;
            view_params["Scale"] = view.scale;
            view_params["X"] = 0.0;
            view_params["Y"] = 0.0;
            view_params["Rotation"] = 0.0;
            
            view_obj->setPropertyByName("Scale", &view_params["Scale"]);
            view_obj->setPropertyByName("X", &view_params["X"]);
            view_obj->setPropertyByName("Y", &view_params["Y"]);
            view_obj->setPropertyByName("Rotation", &view_params["Rotation"]);
            
            if (!view.source_model_id.empty()) {
                App::DocumentObject* source = doc->getObject(view.source_model_id.c_str());
                if (source) {
                    view_obj->setPropertyByName("Source", source);
                }
            }
            
            std::vector<double> direction = parseOrientation(view.orientation);
            if (direction.size() >= 3) {
                view_obj->setPropertyByName("Direction", &direction);
            }
            
            std::vector<App::DocumentObject*> page_views;
            page_views.push_back(view_obj);
            page->setPropertyByName("Views", &page_views);
        }
    }
    
    doc->recompute();
    return true;
#else
    (void)document;
    return false;
#endif
}

bool TechDrawBridge::syncAssociativeLinks(const cad::drawings::DrawingDocument& document) {
#ifdef CAD_USE_FREECAD
    App::Application* app = App::GetApplication();
    if (!app) {
        return false;
    }
    App::Document* doc = app->getActiveDocument();
    if (!doc) {
        return false;
    }
    
    for (const auto& sheet : document.sheets()) {
        for (const auto& view : sheet.views()) {
            if (view.associative && !view.source_model_id.empty()) {
                App::DocumentObject* view_obj = doc->getObject(view.name().c_str());
                if (!view_obj) {
                    continue;
                }
                
                App::DocumentObject* source = doc->getObject(view.source_model_id.c_str());
                if (source) {
                    view_obj->setPropertyByName("Source", source);
                    
                    bool keep_updated = true;
                    view_obj->setPropertyByName("KeepUpdated", &keep_updated);
                    
                    std::map<std::string, bool> link_params;
                    link_params["Associative"] = true;
                    link_params["AutoUpdate"] = true;
                    view_obj->setPropertyByName("LinkParams", &link_params);
                }
            }
        }
    }
    
    doc->recompute();
    return true;
#else
    (void)document;
    return false;
#endif
}

bool TechDrawBridge::syncDimensions(const cad::drawings::DrawingDocument& document) {
#ifdef CAD_USE_FREECAD
    App::Application* app = App::GetApplication();
    if (!app) {
        return false;
    }
    App::Document* doc = app->getActiveDocument();
    if (!doc) {
        return false;
    }
    
    for (const auto& sheet : document.sheets()) {
        for (const auto& view : sheet.views()) {
            App::DocumentObject* view_obj = doc->getObject(view.name().c_str());
            if (!view_obj) {
                continue;
            }
            
            std::vector<App::DocumentObject*> view_dimensions;
            
            for (const auto& dimension : document.dimensions()) {
                App::DocumentObject* dim_obj = doc->addObject("TechDraw::DrawViewDimension", dimension.label.c_str());
                if (!dim_obj) {
                    continue;
                }
                
                std::map<std::string, std::string> dim_params;
                dim_params["FormatSpec"] = std::to_string(dimension.value);
                dim_params["Units"] = dimension.units;
                dim_params["Tolerance"] = dimension.tolerance;
                dim_params["Type"] = "Distance";
                
                dim_obj->setPropertyByName("FormatSpec", &dim_params["FormatSpec"]);
                dim_obj->setPropertyByName("Units", &dim_params["Units"]);
                dim_obj->setPropertyByName("Tolerance", &dim_params["Tolerance"]);
                dim_obj->setPropertyByName("Type", &dim_params["Type"]);
                
                double dim_value = dimension.value;
                dim_obj->setPropertyByName("Value", &dim_value);
                
                view_obj->setPropertyByName("Source", view_obj);
                view_dimensions.push_back(dim_obj);
            }
            
            if (!view_dimensions.empty()) {
                view_obj->setPropertyByName("Dimensions", &view_dimensions);
            }
        }
    }
    
    doc->recompute();
    return true;
#else
    (void)document;
    return false;
#endif
}

std::vector<double> TechDrawBridge::parseOrientation(const std::string& orientation) const {
    std::vector<double> direction(3, 0.0);
    
    if (orientation == "Front" || orientation == "XY") {
        direction[0] = 0.0;
        direction[1] = 0.0;
        direction[2] = 1.0;
    } else if (orientation == "Top" || orientation == "XZ") {
        direction[0] = 0.0;
        direction[1] = -1.0;
        direction[2] = 0.0;
    } else if (orientation == "Right" || orientation == "YZ") {
        direction[0] = 1.0;
        direction[1] = 0.0;
        direction[2] = 0.0;
    } else if (orientation == "Isometric") {
        direction[0] = 1.0;
        direction[1] = 1.0;
        direction[2] = 1.0;
    } else {
        direction[0] = 0.0;
        direction[1] = 0.0;
        direction[2] = 1.0;
    }
    
    double length = std::sqrt(direction[0]*direction[0] + direction[1]*direction[1] + direction[2]*direction[2]);
    if (length > 0.001) {
        direction[0] /= length;
        direction[1] /= length;
        direction[2] /= length;
    }
    
    return direction;
}

std::vector<double> TechDrawBridge::parseCutPlane(const std::string& cut_plane) const {
    std::vector<double> plane(4, 0.0);
    
    if (cut_plane == "XY" || cut_plane == "Front") {
        plane[0] = 0.0;
        plane[1] = 0.0;
        plane[2] = 1.0;
        plane[3] = 0.0;
    } else if (cut_plane == "XZ" || cut_plane == "Top") {
        plane[0] = 0.0;
        plane[1] = -1.0;
        plane[2] = 0.0;
        plane[3] = 0.0;
    } else if (cut_plane == "YZ" || cut_plane == "Right") {
        plane[0] = 1.0;
        plane[1] = 0.0;
        plane[2] = 0.0;
        plane[3] = 0.0;
    } else {
        plane[0] = 0.0;
        plane[1] = 0.0;
        plane[2] = 1.0;
        plane[3] = 0.0;
    }
    
    return plane;
}

bool TechDrawBridge::createBaseView(const std::string& view_name, const std::string& source_id, const std::string& orientation, double scale) {
#ifdef CAD_USE_FREECAD
    App::Application* app = App::GetApplication();
    if (!app) {
        return false;
    }
    App::Document* doc = app->getActiveDocument();
    if (!doc) {
        return false;
    }
    
    App::DocumentObject* view_obj = doc->addObject("TechDraw::DrawViewPart", view_name.c_str());
    if (!view_obj) {
        return false;
    }
    
    std::map<std::string, double> view_params;
    view_params["Scale"] = scale;
    view_params["X"] = 0.0;
    view_params["Y"] = 0.0;
    view_params["Rotation"] = 0.0;
    
    view_obj->setPropertyByName("Scale", &view_params["Scale"]);
    view_obj->setPropertyByName("X", &view_params["X"]);
    view_obj->setPropertyByName("Y", &view_params["Y"]);
    view_obj->setPropertyByName("Rotation", &view_params["Rotation"]);
    
    if (!source_id.empty()) {
        App::DocumentObject* source = doc->getObject(source_id.c_str());
        if (source) {
            view_obj->setPropertyByName("Source", source);
        }
    }
    
    std::vector<double> direction = parseOrientation(orientation);
    if (direction.size() >= 3) {
        view_obj->setPropertyByName("Direction", &direction);
    }
    
    doc->recompute();
    return true;
#else
    (void)view_name;
    (void)source_id;
    (void)orientation;
    (void)scale;
    return false;
#endif
}

bool TechDrawBridge::createSectionView(const std::string& view_name, const std::string& source_id, const std::string& cut_plane, double scale) {
#ifdef CAD_USE_FREECAD
    App::Application* app = App::GetApplication();
    if (!app) {
        return false;
    }
    App::Document* doc = app->getActiveDocument();
    if (!doc) {
        return false;
    }
    
    App::DocumentObject* view_obj = doc->addObject("TechDraw::DrawViewSection", view_name.c_str());
    if (!view_obj) {
        return false;
    }
    
    std::map<std::string, double> view_params;
    view_params["Scale"] = scale;
    view_params["X"] = 0.0;
    view_params["Y"] = 0.0;
    
    view_obj->setPropertyByName("Scale", &view_params["Scale"]);
    view_obj->setPropertyByName("X", &view_params["X"]);
    view_obj->setPropertyByName("Y", &view_params["Y"]);
    
    if (!source_id.empty()) {
        App::DocumentObject* source = doc->getObject(source_id.c_str());
        if (source) {
            view_obj->setPropertyByName("Source", source);
        }
    }
    
    std::vector<double> plane = parseCutPlane(cut_plane);
    if (plane.size() >= 4) {
        view_obj->setPropertyByName("SectionPlane", &plane);
    }
    
    doc->recompute();
    return true;
#else
    (void)view_name;
    (void)source_id;
    (void)cut_plane;
    (void)scale;
    return false;
#endif
}

bool TechDrawBridge::createDetailView(const std::string& view_name, const std::string& source_id, double center_x, double center_y, double radius, double scale) {
#ifdef CAD_USE_FREECAD
    App::Application* app = App::GetApplication();
    if (!app) {
        return false;
    }
    App::Document* doc = app->getActiveDocument();
    if (!doc) {
        return false;
    }
    
    App::DocumentObject* view_obj = doc->addObject("TechDraw::DrawViewDetail", view_name.c_str());
    if (!view_obj) {
        return false;
    }
    
    std::map<std::string, double> view_params;
    view_params["Scale"] = scale;
    view_params["X"] = center_x;
    view_params["Y"] = center_y;
    view_params["Radius"] = radius;
    
    view_obj->setPropertyByName("Scale", &view_params["Scale"]);
    view_obj->setPropertyByName("X", &view_params["X"]);
    view_obj->setPropertyByName("Y", &view_params["Y"]);
    view_obj->setPropertyByName("Radius", &view_params["Radius"]);
    
    if (!source_id.empty()) {
        App::DocumentObject* source = doc->getObject(source_id.c_str());
        if (source) {
            view_obj->setPropertyByName("Source", source);
        }
    }
    
    doc->recompute();
    return true;
#else
    (void)view_name;
    (void)source_id;
    (void)center_x;
    (void)center_y;
    (void)radius;
    (void)scale;
    return false;
#endif
}

bool TechDrawBridge::addDimension(const std::string& view_name, const std::string& dim_label, double value, double x1, double y1, double x2, double y2) {
#ifdef CAD_USE_FREECAD
    App::Application* app = App::GetApplication();
    if (!app) {
        return false;
    }
    App::Document* doc = app->getActiveDocument();
    if (!doc) {
        return false;
    }
    
    App::DocumentObject* view_obj = doc->getObject(view_name.c_str());
    if (!view_obj) {
        return false;
    }
    
    App::DocumentObject* dim_obj = doc->addObject("TechDraw::DrawViewDimension", dim_label.c_str());
    if (!dim_obj) {
        return false;
    }
    
    double dim_value = value;
    dim_obj->setPropertyByName("Value", &dim_value);
    dim_obj->setPropertyByName("Type", "Distance");
    
    std::vector<double> start_point = {x1, y1, 0.0};
    std::vector<double> end_point = {x2, y2, 0.0};
    dim_obj->setPropertyByName("StartPoint", &start_point);
    dim_obj->setPropertyByName("EndPoint", &end_point);
    dim_obj->setPropertyByName("Source", view_obj);
    
    doc->recompute();
    return true;
#else
    (void)view_name;
    (void)dim_label;
    (void)value;
    (void)x1;
    (void)y1;
    (void)x2;
    (void)y2;
    return false;
#endif
}

bool TechDrawBridge::addBomTable(const std::string& sheet_name, const std::vector<std::tuple<std::string, int, std::string>>& bom_items) {
#ifdef CAD_USE_FREECAD
    App::Application* app = App::GetApplication();
    if (!app) {
        return false;
    }
    App::Document* doc = app->getActiveDocument();
    if (!doc) {
        return false;
    }
    
    App::DocumentObject* sheet_obj = doc->getObject(sheet_name.c_str());
    if (!sheet_obj) {
        return false;
    }
    
    App::DocumentObject* bom_obj = doc->addObject("TechDraw::DrawViewTable", "BOM_Table");
    if (!bom_obj) {
        return false;
    }
    
    std::vector<std::vector<std::string>> table_data;
    table_data.push_back({"Part Name", "Quantity", "Part Number"});
    
    for (const auto& item : bom_items) {
        std::vector<std::string> row;
        row.push_back(std::get<0>(item));
        row.push_back(std::to_string(std::get<1>(item)));
        row.push_back(std::get<2>(item));
        table_data.push_back(row);
    }
    
    bom_obj->setPropertyByName("Data", &table_data);
    bom_obj->setPropertyByName("Source", sheet_obj);
    
    double bom_x = 150.0;
    double bom_y = 50.0;
    bom_obj->setPropertyByName("X", &bom_x);
    bom_obj->setPropertyByName("Y", &bom_y);
    
    doc->recompute();
    return true;
#else
    (void)sheet_name;
    (void)bom_items;
    return false;
#endif
}

bool TechDrawBridge::applySheetTemplate(const std::string& sheet_name, const std::string& template_name) {
#ifdef CAD_USE_FREECAD
    App::Application* app = App::GetApplication();
    if (!app) {
        return false;
    }
    App::Document* doc = app->getActiveDocument();
    if (!doc) {
        return false;
    }
    
    App::DocumentObject* sheet_obj = doc->getObject(sheet_name.c_str());
    if (!sheet_obj) {
        return false;
    }
    
    std::map<std::string, double> template_params;
    
    if (template_name == "ISO" || template_name == "ISO_A4") {
        template_params["Width"] = 210.0;
        template_params["Height"] = 297.0;
    } else if (template_name == "ANSI" || template_name == "ANSI_A") {
        template_params["Width"] = 216.0;
        template_params["Height"] = 279.0;
    } else if (template_name == "JIS" || template_name == "JIS_A4") {
        template_params["Width"] = 210.0;
        template_params["Height"] = 297.0;
    } else {
        template_params["Width"] = 210.0;
        template_params["Height"] = 297.0;
    }
    
    sheet_obj->setPropertyByName("Width", &template_params["Width"]);
    sheet_obj->setPropertyByName("Height", &template_params["Height"]);
    sheet_obj->setPropertyByName("Template", template_name.c_str());
    
    doc->recompute();
    return true;
#else
    (void)sheet_name;
    (void)template_name;
    return false;
#endif
}

}  // namespace core
}  // namespace cad
