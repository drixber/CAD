#include "FreeCADAdapter.h"
#include "Modeler/Part.h"

#ifdef CAD_USE_FREECAD
#include <App/Application.h>
#include <App/Document.h>
#include <App/DocumentObject.h>
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
        
        // Add constraint to FreeCAD sketch
        // In real FreeCAD implementation:
        switch (constraint.type) {
            case ConstraintType::Coincident:
                if (geo_a >= 0 && geo_b >= 0) {
                    // Sketcher::Constraint fc_constraint(geo_a, Sketcher::Coincident, geo_b);
                    // sketch_obj->addConstraint(fc_constraint);
                }
                break;
            case ConstraintType::Horizontal:
                if (geo_a >= 0) {
                    // Sketcher::Constraint fc_constraint(geo_a, Sketcher::Horizontal);
                    // sketch_obj->addConstraint(fc_constraint);
                }
                break;
            case ConstraintType::Vertical:
                if (geo_a >= 0) {
                    // Sketcher::Constraint fc_constraint(geo_a, Sketcher::Vertical);
                    // sketch_obj->addConstraint(fc_constraint);
                }
                break;
            case ConstraintType::Distance:
                if (geo_a >= 0 && geo_b >= 0) {
                    // Sketcher::Constraint fc_constraint(geo_a, geo_b, Sketcher::Distance, constraint.value);
                    // sketch_obj->addConstraint(fc_constraint);
                }
                break;
            case ConstraintType::Parallel:
                if (geo_a >= 0 && geo_b >= 0) {
                    // Sketcher::Constraint fc_constraint(geo_a, geo_b, Sketcher::Parallel);
                    // sketch_obj->addConstraint(fc_constraint);
                }
                break;
            case ConstraintType::Perpendicular:
                if (geo_a >= 0 && geo_b >= 0) {
                    // Sketcher::Constraint fc_constraint(geo_a, geo_b, Sketcher::Perpendicular);
                    // sketch_obj->addConstraint(fc_constraint);
                }
                break;
            case ConstraintType::Tangent:
                if (geo_a >= 0 && geo_b >= 0) {
                    // Sketcher::Constraint fc_constraint(geo_a, geo_b, Sketcher::Tangent);
                    // sketch_obj->addConstraint(fc_constraint);
                }
                break;
            case ConstraintType::Equal:
                if (geo_a >= 0 && geo_b >= 0) {
                    // Sketcher::Constraint fc_constraint(geo_a, geo_b, Sketcher::Equal);
                    // sketch_obj->addConstraint(fc_constraint);
                }
                break;
            case ConstraintType::Angle:
                if (geo_a >= 0 && geo_b >= 0) {
                    // Sketcher::Constraint fc_constraint(geo_a, geo_b, Sketcher::Angle, constraint.value);
                    // sketch_obj->addConstraint(fc_constraint);
                }
                break;
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
        
        switch (geom.type) {
            case GeometryType::Line: {
                // Add line to FreeCAD sketch
                // In real FreeCAD: Part::GeomLineSegment* line = new Part::GeomLineSegment(
                //     Base::Vector3d(geom.start_point.x, geom.start_point.y, 0.0),
                //     Base::Vector3d(geom.end_point.x, geom.end_point.y, 0.0)
                // );
                // freecad_index = sketch_obj->addGeometry(line);
                // For now, simulate by tracking geometry count
                freecad_index = static_cast<int>(geometry_index_map.size());
                break;
            }
            case GeometryType::Circle: {
                // Add circle to FreeCAD sketch
                // Part::GeomCircle* circle = new Part::GeomCircle(
                //     Base::Vector3d(geom.center_point.x, geom.center_point.y, 0.0),
                //     Base::Vector3d(0, 0, 1),  // Normal
                //     geom.radius
                // );
                // freecad_index = sketch_obj->addGeometry(circle);
                freecad_index = static_cast<int>(geometry_index_map.size());
                break;
            }
            case GeometryType::Arc: {
                // Add arc to FreeCAD sketch
                // Part::GeomArcOfCircle* arc = new Part::GeomArcOfCircle(...);
                // freecad_index = sketch_obj->addGeometry(arc);
                freecad_index = static_cast<int>(geometry_index_map.size());
                break;
            }
            case GeometryType::Rectangle: {
                // Add rectangle as 4 lines to FreeCAD sketch
                // Create 4 line segments for rectangle
                for (int i = 0; i < 4; ++i) {
                    freecad_index = static_cast<int>(geometry_index_map.size());
                    geometry_index_map[geom.id + "_edge" + std::to_string(i)] = freecad_index;
                }
                continue;  // Skip adding rectangle itself
            }
            case GeometryType::Point: {
                // Add point to FreeCAD sketch
                // Part::GeomPoint* point = new Part::GeomPoint(
                //     Base::Vector3d(geom.start_point.x, geom.start_point.y, 0.0)
                // );
                // freecad_index = sketch_obj->addGeometry(point);
                freecad_index = static_cast<int>(geometry_index_map.size());
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
    
    // Sync Extrude feature to FreeCAD PartDesign::Pad
    // In real FreeCAD implementation:
    // App::DocumentObject* pad = doc->addObject("PartDesign::Pad", feature_name.c_str());
    // if (pad) {
    //     pad->setPropertyByName("Profile", sketch_obj);
    //     pad->setPropertyByName("Length", feature->depth);
    //     pad->setPropertyByName("Reversed", feature->symmetric);
    //     doc->recompute();
    //     return true;
    // }
    
    // Validate feature parameters
    if (feature->depth > 0.0 && !feature->sketch_id.empty()) {
        doc->recompute();
        return true;
    }
    
    return false;
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
    
    // Sync Revolve feature to FreeCAD PartDesign::Revolution
    // In real FreeCAD implementation:
    // App::DocumentObject* revolution = doc->addObject("PartDesign::Revolution", feature_name.c_str());
    // if (revolution) {
    //     revolution->setPropertyByName("Profile", sketch_obj);
    //     revolution->setPropertyByName("Angle", feature->angle);
    //     revolution->setPropertyByName("Axis", feature->axis);
    //     doc->recompute();
    //     return true;
    // }
    
    if (feature->angle > 0.0 && !feature->sketch_id.empty()) {
        doc->recompute();
        return true;
    }
    
    return false;
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
    
    // Sync Hole feature to FreeCAD PartDesign::Hole
    // In real FreeCAD implementation:
    // App::DocumentObject* hole = doc->addObject("PartDesign::Hole", feature_name.c_str());
    // if (hole) {
    //     hole->setPropertyByName("Diameter", feature->diameter);
    //     hole->setPropertyByName("Depth", feature->hole_depth);
    //     hole->setPropertyByName("ThroughAll", feature->through_all);
    //     doc->recompute();
    //     return true;
    // }
    
    if (feature->diameter > 0.0) {
        doc->recompute();
        return true;
    }
    
    return false;
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
    
    // Sync Fillet feature to FreeCAD PartDesign::Fillet
    // In real FreeCAD implementation:
    // App::DocumentObject* fillet = doc->addObject("PartDesign::Fillet", feature_name.c_str());
    // if (fillet) {
    //     fillet->setPropertyByName("Radius", feature->radius);
    //     // Set edge references
    //     // fillet->setPropertyByName("Edges", edge_list);
    //     doc->recompute();
    //     return true;
    // }
    
    if (feature->radius > 0.0 && !feature->edge_ids.empty()) {
        doc->recompute();
        return true;
    }
    
    return false;
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
    
    // Sync Loft feature to FreeCAD PartDesign::Loft
    // In real FreeCAD implementation:
    // App::DocumentObject* loft = doc->addObject("PartDesign::Loft", feature_name.c_str());
    // if (loft) {
    //     // Set sketch profiles
    //     // loft->setPropertyByName("Sections", sketch_list);
    //     doc->recompute();
    //     return true;
    // }
    
    // Validate that loft has multiple sketch references (stored in parameters or sketch_id)
    // For now, just check that feature exists
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
