#include "FreeCADAdapter.h"
#include "Modeler/Part.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifdef CAD_USE_FREECAD
#include <App/Application.h>
#include <App/Document.h>
#include <App/DocumentObject.h>
#include <cmath>
#endif

namespace cad {
namespace core {

FreeCADAdapter::FreeCADAdapter() = default;

bool FreeCADAdapter::initializeSession() {
#ifdef CAD_USE_FREECAD
    initialized_ = (App::GetApplication() != nullptr);
    return initialized_;
#else
    initialized_ = false;
    return false;
#endif
}

bool FreeCADAdapter::createDocument(const std::string& name) {
    if (!initialized_) {
        return false;
    }
#ifdef CAD_USE_FREECAD
    App::Application* app = App::GetApplication();
    if (!app) {
        return false;
    }
    App::Document* doc = app->newDocument(name.c_str(), name.c_str(), true, false);
    if (!doc) {
        return false;
    }
    active_document_ = doc->getName();
    return true;
#else
    (void)name;
    return false;
#endif
}

const std::string& FreeCADAdapter::activeDocument() const {
    return active_document_;
}

bool FreeCADAdapter::createSketchStub(const std::string& name) const {
#ifdef CAD_USE_FREECAD
    App::Application* app = App::GetApplication();
    if (!app) {
        return false;
    }
    App::Document* doc = app->getActiveDocument();
    if (!doc) {
        return false;
    }
    App::DocumentObject* obj = doc->addObject("Sketcher::SketchObject", name.c_str());
    if (!obj) {
        return false;
    }
    doc->recompute();
    return true;
#else
    (void)name;
    return false;
#endif
}

bool FreeCADAdapter::syncSketch(const Sketch& sketch) const {
#ifdef CAD_USE_FREECAD
    if (sketch.name().empty()) {
        return false;
    }
    if (!createSketchStub(sketch.name())) {
        return false;
    }
    return true;
#else
    (void)sketch;
    return false;
#endif
}

bool FreeCADAdapter::syncConstraints(const Sketch& sketch) const {
#ifdef CAD_USE_FREECAD
    if (sketch.constraints().empty()) {
        return true;
    }
    
    App::Application* app = App::GetApplication();
    if (!app) {
        return false;
    }
    App::Document* doc = app->getActiveDocument();
    if (!doc) {
        return false;
    }
    
    App::DocumentObject* sketch_obj = doc->getObject(sketch.name().c_str());
    if (!sketch_obj) {
        return false;
    }
    
    // Map constraints to FreeCAD Sketcher constraints
    // First, build geometry index map (would be done during geometry sync)
    std::map<std::string, int> geometry_index_map;
    int current_index = 0;
    for (const auto& geom : sketch.geometry()) {
        geometry_index_map[geom.id] = current_index++;
    }
    
    for (const auto& constraint : sketch.constraints()) {
        int geo_a = -1;
        int geo_b = -1;
        
        // Map geometry IDs to FreeCAD indices
        auto it_a = geometry_index_map.find(constraint.a);
        if (it_a != geometry_index_map.end()) {
            geo_a = it_a->second;
        }
        
        auto it_b = geometry_index_map.find(constraint.b);
        if (it_b != geometry_index_map.end()) {
            geo_b = it_b->second;
        }
        
        // Skip if geometry not found
        if (geo_a < 0 && geo_b < 0) {
            continue;
        }
        
        int constraint_type_code = -1;
        double constraint_value = constraint.value;
        
        switch (constraint.type) {
            case ConstraintType::Coincident:
                if (geo_a >= 0 && geo_b >= 0) {
                    constraint_type_code = 0;
                }
                break;
            case ConstraintType::Horizontal:
                if (geo_a >= 0) {
                    constraint_type_code = 1;
                }
                break;
            case ConstraintType::Vertical:
                if (geo_a >= 0) {
                    constraint_type_code = 2;
                }
                break;
            case ConstraintType::Distance:
                if (geo_a >= 0 && geo_b >= 0) {
                    constraint_type_code = 3;
                }
                break;
            case ConstraintType::Parallel:
                if (geo_a >= 0 && geo_b >= 0) {
                    constraint_type_code = 4;
                }
                break;
            case ConstraintType::Perpendicular:
                if (geo_a >= 0 && geo_b >= 0) {
                    constraint_type_code = 5;
                }
                break;
            case ConstraintType::Tangent:
                if (geo_a >= 0 && geo_b >= 0) {
                    constraint_type_code = 6;
                }
                break;
            case ConstraintType::Equal:
                if (geo_a >= 0 && geo_b >= 0) {
                    constraint_type_code = 7;
                }
                break;
            case ConstraintType::Angle:
                if (geo_a >= 0 && geo_b >= 0) {
                    constraint_type_code = 8;
                }
                break;
        }
        
        if (constraint_type_code >= 0) {
            std::map<std::string, int> constraint_map;
            constraint_map["type"] = constraint_type_code;
            constraint_map["geo_a"] = geo_a;
            constraint_map["geo_b"] = geo_b;
            constraint_map["value"] = static_cast<int>(constraint_value * 1000);
            
            std::string constraint_key = "constraint_" + std::to_string(constraint_map.size());
            sketch_obj->setPropertyByName(constraint_key.c_str(), &constraint_map);
        }
    }
    
    doc->recompute();
    return true;
#else
    (void)sketch;
    return false;
#endif
}

bool FreeCADAdapter::syncGeometry(const Sketch& sketch) const {
#ifdef CAD_USE_FREECAD
    if (sketch.geometry().empty()) {
        return true;
    }
    
    App::Application* app = App::GetApplication();
    if (!app) {
        return false;
    }
    App::Document* doc = app->getActiveDocument();
    if (!doc) {
        return false;
    }
    
    // Find or create sketch object
    App::DocumentObject* sketch_obj = doc->getObject(sketch.name().c_str());
    if (!sketch_obj) {
        if (!createSketchStub(sketch.name())) {
            return false;
        }
        sketch_obj = doc->getObject(sketch.name().c_str());
    }
    
    if (!sketch_obj) {
        return false;
    }
    
    // Sync geometry entities to FreeCAD Sketcher
    // Map our geometry types to FreeCAD geometry
    std::map<std::string, int> geometry_index_map;  // Map our geometry IDs to FreeCAD indices
    
    for (const auto& geom : sketch.geometry()) {
        int freecad_index = -1;
        
        std::map<std::string, double> geometry_params;
        geometry_params["type"] = static_cast<double>(geom.type);
        geometry_params["start_x"] = geom.start_point.x;
        geometry_params["start_y"] = geom.start_point.y;
        geometry_params["end_x"] = geom.end_point.x;
        geometry_params["end_y"] = geom.end_point.y;
        geometry_params["center_x"] = geom.center_point.x;
        geometry_params["center_y"] = geom.center_point.y;
        geometry_params["radius"] = geom.radius;
        geometry_params["start_angle"] = geom.start_angle;
        geometry_params["end_angle"] = geom.end_angle;
        geometry_params["width"] = geom.width;
        geometry_params["height"] = geom.height;
        
        switch (geom.type) {
            case GeometryType::Line: {
                geometry_params["length"] = std::sqrt(
                    (geom.end_point.x - geom.start_point.x) * (geom.end_point.x - geom.start_point.x) +
                    (geom.end_point.y - geom.start_point.y) * (geom.end_point.y - geom.start_point.y)
                );
                freecad_index = static_cast<int>(geometry_index_map.size());
                std::string geom_key = "geometry_" + std::to_string(freecad_index);
                sketch_obj->setPropertyByName(geom_key.c_str(), &geometry_params);
                break;
            }
            case GeometryType::Circle: {
                geometry_params["circumference"] = 2.0 * M_PI * geom.radius;
                geometry_params["area"] = M_PI * geom.radius * geom.radius;
                freecad_index = static_cast<int>(geometry_index_map.size());
                std::string geom_key = "geometry_" + std::to_string(freecad_index);
                sketch_obj->setPropertyByName(geom_key.c_str(), &geometry_params);
                break;
            }
            case GeometryType::Arc: {
                double angle_span = geom.end_angle - geom.start_angle;
                geometry_params["arc_length"] = geom.radius * angle_span * M_PI / 180.0;
                freecad_index = static_cast<int>(geometry_index_map.size());
                std::string geom_key = "geometry_" + std::to_string(freecad_index);
                sketch_obj->setPropertyByName(geom_key.c_str(), &geometry_params);
                break;
            }
            case GeometryType::Rectangle: {
                for (int i = 0; i < 4; ++i) {
                    freecad_index = static_cast<int>(geometry_index_map.size());
                    geometry_params["edge_index"] = static_cast<double>(i);
                    std::string edge_key = "geometry_" + std::to_string(freecad_index) + "_edge" + std::to_string(i);
                    sketch_obj->setPropertyByName(edge_key.c_str(), &geometry_params);
                    geometry_index_map[geom.id + "_edge" + std::to_string(i)] = freecad_index;
                }
                continue;
            }
            case GeometryType::Point: {
                freecad_index = static_cast<int>(geometry_index_map.size());
                std::string geom_key = "geometry_" + std::to_string(freecad_index);
                sketch_obj->setPropertyByName(geom_key.c_str(), &geometry_params);
                break;
            }
        }
        
        if (freecad_index >= 0) {
            geometry_index_map[geom.id] = freecad_index;
        }
    }
    
    doc->recompute();
    return true;
#else
    (void)sketch;
    return false;
#endif
}

bool FreeCADAdapter::createDrawingStub(const std::string& name) const {
#ifdef CAD_USE_FREECAD
    App::Application* app = App::GetApplication();
    if (!app) {
        return false;
    }
    App::Document* doc = app->getActiveDocument();
    if (!doc) {
        return false;
    }
    App::DocumentObject* obj = doc->addObject("TechDraw::DrawPage", name.c_str());
    if (!obj) {
        return false;
    }
    doc->recompute();
    return true;
#else
    (void)name;
    return false;
#endif
}

Part FreeCADAdapter::buildPartFromSketch(const Sketch& sketch) const {
#ifdef CAD_USE_FREECAD
    (void)sketch;
    App::Application* app = App::GetApplication();
    if (!app) {
        return Part("FreeCADPart");
    }
    App::Document* doc = app->getActiveDocument();
    if (!doc) {
        return Part("FreeCADPart");
    }
    App::DocumentObject* obj = doc->addObject("Part::Box", "Box");
    if (obj) {
        doc->recompute();
        return Part(obj->getNameInDocument());
    }
    return Part("FreeCADPart");
#else
    Modeler modeler;
    return modeler.createPart(sketch);
#endif
}

bool FreeCADAdapter::isAvailable() const {
    #ifdef CAD_USE_FREECAD
    return true;
    #else
    return false;
    #endif
}

bool FreeCADAdapter::syncExtrude(const Part& part, const std::string& feature_name) const {
#ifdef CAD_USE_FREECAD
    App::Application* app = App::GetApplication();
    if (!app) {
        return false;
    }
    App::Document* doc = app->getActiveDocument();
    if (!doc) {
        return false;
    }
    
    // Find the Extrude feature in the part
    const Feature* feature = part.findFeature(feature_name);
    if (!feature || feature->type != FeatureType::Extrude) {
        return false;
    }
    
    App::DocumentObject* sketch_obj = doc->getObject(feature->sketch_id.c_str());
    if (!sketch_obj) {
        return false;
    }
    
    App::DocumentObject* pad = doc->addObject("PartDesign::Pad", feature_name.c_str());
    if (!pad) {
        return false;
    }
    
    std::map<std::string, double> pad_params;
    pad_params["Length"] = feature->depth;
    pad_params["Reversed"] = feature->symmetric ? 1.0 : 0.0;
    pad_params["Type"] = 0.0;
    pad_params["UpToFace"] = 0.0;
    pad_params["Offset"] = 0.0;
    
    pad->setPropertyByName("Length", &pad_params["Length"]);
    pad->setPropertyByName("Reversed", &pad_params["Reversed"]);
    pad->setPropertyByName("Profile", sketch_obj);
    
    doc->recompute();
    return true;
#else
    (void)part;
    (void)feature_name;
    return false;
#endif
}

bool FreeCADAdapter::syncRevolve(const Part& part, const std::string& feature_name) const {
#ifdef CAD_USE_FREECAD
    App::Application* app = App::GetApplication();
    if (!app) {
        return false;
    }
    App::Document* doc = app->getActiveDocument();
    if (!doc) {
        return false;
    }
    
    const Feature* feature = part.findFeature(feature_name);
    if (!feature || feature->type != FeatureType::Revolve) {
        return false;
    }
    
    App::DocumentObject* sketch_obj = doc->getObject(feature->sketch_id.c_str());
    if (!sketch_obj) {
        return false;
    }
    
    App::DocumentObject* revolution = doc->addObject("PartDesign::Revolution", feature_name.c_str());
    if (!revolution) {
        return false;
    }
    
    std::map<std::string, double> rev_params;
    rev_params["Angle"] = feature->angle;
    rev_params["AxisX"] = (feature->axis == "X") ? 1.0 : 0.0;
    rev_params["AxisY"] = (feature->axis == "Y") ? 1.0 : 0.0;
    rev_params["AxisZ"] = (feature->axis == "Z") ? 1.0 : 0.0;
    
    revolution->setPropertyByName("Angle", &rev_params["Angle"]);
    revolution->setPropertyByName("Axis", &rev_params);
    revolution->setPropertyByName("Profile", sketch_obj);
    
    doc->recompute();
    return true;
#else
    (void)part;
    (void)feature_name;
    return false;
#endif
}

bool FreeCADAdapter::syncHole(const Part& part, const std::string& feature_name) const {
#ifdef CAD_USE_FREECAD
    App::Application* app = App::GetApplication();
    if (!app) {
        return false;
    }
    App::Document* doc = app->getActiveDocument();
    if (!doc) {
        return false;
    }
    
    const Feature* feature = part.findFeature(feature_name);
    if (!feature || feature->type != FeatureType::Hole) {
        return false;
    }
    
    App::DocumentObject* hole = doc->addObject("PartDesign::Hole", feature_name.c_str());
    if (!hole) {
        return false;
    }
    
    std::map<std::string, double> hole_params;
    hole_params["Diameter"] = feature->diameter;
    hole_params["Depth"] = feature->through_all ? -1.0 : feature->hole_depth;
    hole_params["ThroughAll"] = feature->through_all ? 1.0 : 0.0;
    hole_params["Threaded"] = 0.0;
    hole_params["ThreadType"] = 0.0;
    hole_params["ThreadSize"] = 0.0;
    hole_params["ThreadClass"] = 0.0;
    hole_params["ThreadFit"] = 0.0;
    hole_params["ThreadDirection"] = 0.0;
    hole_params["HoleCutType"] = 0.0;
    hole_params["HoleCutDiameter"] = 0.0;
    hole_params["HoleCutDepth"] = 0.0;
    hole_params["HoleCutCountersinkAngle"] = 0.0;
    hole_params["Tapered"] = 0.0;
    hole_params["TaperedAngle"] = 0.0;
    
    hole->setPropertyByName("Diameter", &hole_params["Diameter"]);
    hole->setPropertyByName("Depth", &hole_params["Depth"]);
    hole->setPropertyByName("ThroughAll", &hole_params["ThroughAll"]);
    
    doc->recompute();
    return true;
#else
    (void)part;
    (void)feature_name;
    return false;
#endif
}

bool FreeCADAdapter::syncFillet(const Part& part, const std::string& feature_name) const {
#ifdef CAD_USE_FREECAD
    App::Application* app = App::GetApplication();
    if (!app) {
        return false;
    }
    App::Document* doc = app->getActiveDocument();
    if (!doc) {
        return false;
    }
    
    const Feature* feature = part.findFeature(feature_name);
    if (!feature || feature->type != FeatureType::Fillet) {
        return false;
    }
    
    App::DocumentObject* fillet = doc->addObject("PartDesign::Fillet", feature_name.c_str());
    if (!fillet) {
        return false;
    }
    
    std::map<std::string, double> fillet_params;
    fillet_params["Radius"] = feature->radius;
    fillet_params["EdgeCount"] = static_cast<double>(feature->edge_ids.size());
    
    fillet->setPropertyByName("Radius", &fillet_params["Radius"]);
    
    std::vector<std::string> edge_refs = feature->edge_ids;
    fillet->setPropertyByName("Edges", &edge_refs);
    
    doc->recompute();
    return true;
#else
    (void)part;
    (void)feature_name;
    return false;
#endif
}

bool FreeCADAdapter::syncLoft(const Part& part, const std::string& feature_name) const {
#ifdef CAD_USE_FREECAD
    App::Application* app = App::GetApplication();
    if (!app) {
        return false;
    }
    App::Document* doc = app->getActiveDocument();
    if (!doc) {
        return false;
    }
    
    const Feature* feature = part.findFeature(feature_name);
    if (!feature || feature->type != FeatureType::Loft) {
        return false;
    }
    
    std::vector<App::DocumentObject*> sketch_objects;
    std::vector<std::string> sketch_ids;
    
    if (!feature->sketch_id.empty()) {
        sketch_ids.push_back(feature->sketch_id);
    }
    
    auto it = feature->parameters.find("sketch_ids");
    if (it != feature->parameters.end()) {
        int count = static_cast<int>(it->second);
        for (int i = 0; i < count; ++i) {
            std::string param_key = "sketch_" + std::to_string(i);
            auto sketch_it = feature->parameters.find(param_key);
            if (sketch_it != feature->parameters.end()) {
                int sketch_index = static_cast<int>(sketch_it->second);
                std::string sketch_name = "Sketch" + std::to_string(sketch_index);
                App::DocumentObject* sketch_obj = doc->getObject(sketch_name.c_str());
                if (sketch_obj) {
                    sketch_objects.push_back(sketch_obj);
                }
            }
        }
    }
    
    if (sketch_objects.empty()) {
        App::DocumentObject* sketch_obj = doc->getObject(feature->sketch_id.c_str());
        if (sketch_obj) {
            sketch_objects.push_back(sketch_obj);
        }
    }
    
    if (sketch_objects.empty()) {
        return false;
    }
    
    App::DocumentObject* loft = doc->addObject("PartDesign::Loft", feature_name.c_str());
    if (!loft) {
        return false;
    }
    
    std::map<std::string, double> loft_params;
    loft_params["Ruled"] = 0.0;
    loft_params["Closed"] = 0.0;
    loft_params["SectionCount"] = static_cast<double>(sketch_objects.size());
    
    loft->setPropertyByName("Sections", &sketch_objects);
    loft->setPropertyByName("Ruled", &loft_params["Ruled"]);
    loft->setPropertyByName("Closed", &loft_params["Closed"]);
    
    doc->recompute();
    return true;
#else
    (void)part;
    (void)feature_name;
    return false;
#endif
}

}  // namespace core
}  // namespace cad
