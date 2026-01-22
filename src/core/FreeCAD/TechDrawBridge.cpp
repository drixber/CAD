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

}  // namespace core
}  // namespace cad
