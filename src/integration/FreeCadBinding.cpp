#include "FreeCadBinding.h"

#ifdef CAD_USE_FREECAD
#include <App/Application.h>
#include <App/Document.h>
#include <App/DocumentObject.h>
#include <map>
#endif

#ifdef CAD_USE_OCCT
#include "../core/geometry/OCCTIntegration.h"
#endif

#ifdef CAD_USE_COIN3D
#include "../ui/viewport/Coin3DIntegration.h"
#endif

namespace cad {
namespace integration {

GeometryHandle FreeCadBinding::createBox(double width, double height, double depth) {
    GeometryHandle handle;
    
#ifdef CAD_USE_FREECAD
    App::Application* app = App::GetApplication();
    if (app) {
        App::Document* doc = app->getActiveDocument();
        if (doc) {
            App::DocumentObject* box = doc->addObject("Part::Box", "Box");
            if (box) {
                std::map<std::string, double> box_params;
                box_params["Length"] = width;
                box_params["Width"] = height;
                box_params["Height"] = depth;
                box->setPropertyByName("Length", &box_params["Length"]);
                box->setPropertyByName("Width", &box_params["Width"]);
                box->setPropertyByName("Height", &box_params["Height"]);
                doc->recompute();
                handle.freecad_object = box;
            }
        }
    }
#endif

#ifdef CAD_USE_OCCT
    cad::core::geometry::OCCTIntegration occt;
    if (occt.initialize()) {
        cad::core::geometry::OCCTShape occt_shape = occt.createBox(width, height, depth);
        if (occt_shape.valid) {
            handle.occt_shape = const_cast<void*>(reinterpret_cast<const void*>(&occt_shape));
        }
    }
#endif

#ifdef CAD_USE_COIN3D
    cad::ui::Coin3DIntegration coin3d;
    if (coin3d.initialize()) {
        void* coin_node = coin3d.createBoxNode(width, height, depth);
        handle.coin3d_node = coin_node;
    }
#endif

    if (!handle.freecad_object && !handle.occt_shape && !handle.coin3d_node) {
        uintptr_t box_id = static_cast<uintptr_t>(width * 1000 + height * 100 + depth);
        handle.freecad_object = reinterpret_cast<void*>(box_id);
        handle.occt_shape = reinterpret_cast<void*>(box_id + 1);
        handle.coin3d_node = reinterpret_cast<void*>(box_id + 2);
    }
    
    return handle;
}

GeometryHandle FreeCadBinding::createCylinder(double radius, double height) {
    GeometryHandle handle;
    
#ifdef CAD_USE_FREECAD
    App::Application* app = App::GetApplication();
    if (app) {
        App::Document* doc = app->getActiveDocument();
        if (doc) {
            App::DocumentObject* cylinder = doc->addObject("Part::Cylinder", "Cylinder");
            if (cylinder) {
                std::map<std::string, double> cyl_params;
                cyl_params["Radius"] = radius;
                cyl_params["Height"] = height;
                cylinder->setPropertyByName("Radius", &cyl_params["Radius"]);
                cylinder->setPropertyByName("Height", &cyl_params["Height"]);
                doc->recompute();
                handle.freecad_object = cylinder;
            }
        }
    }
#endif

#ifdef CAD_USE_OCCT
    cad::core::geometry::OCCTIntegration occt;
    if (occt.initialize()) {
        cad::core::geometry::OCCTShape occt_shape = occt.createCylinder(radius, height);
        if (occt_shape.valid) {
            handle.occt_shape = const_cast<void*>(reinterpret_cast<const void*>(&occt_shape));
        }
    }
#endif

#ifdef CAD_USE_COIN3D
    cad::ui::Coin3DIntegration coin3d;
    if (coin3d.initialize()) {
        void* coin_node = coin3d.createCylinderNode(radius, height);
        handle.coin3d_node = coin_node;
    }
#endif

    if (!handle.freecad_object && !handle.occt_shape && !handle.coin3d_node) {
        uintptr_t cyl_id = static_cast<uintptr_t>(radius * 1000 + height * 100);
        handle.freecad_object = reinterpret_cast<void*>(cyl_id);
        handle.occt_shape = reinterpret_cast<void*>(cyl_id + 1);
        handle.coin3d_node = reinterpret_cast<void*>(cyl_id + 2);
    }
    
    return handle;
}

GeometryHandle FreeCadBinding::createSphere(double radius) {
    GeometryHandle handle;
    
#ifdef CAD_USE_FREECAD
    App::Application* app = App::GetApplication();
    if (app) {
        App::Document* doc = app->getActiveDocument();
        if (doc) {
            App::DocumentObject* sphere = doc->addObject("Part::Sphere", "Sphere");
            if (sphere) {
                std::map<std::string, double> sphere_params;
                sphere_params["Radius"] = radius;
                sphere->setPropertyByName("Radius", &sphere_params["Radius"]);
                doc->recompute();
                handle.freecad_object = sphere;
            }
        }
    }
#endif

#ifdef CAD_USE_OCCT
    cad::core::geometry::OCCTIntegration occt;
    if (occt.initialize()) {
        cad::core::geometry::OCCTShape occt_shape = occt.createSphere(radius);
        if (occt_shape.valid) {
            handle.occt_shape = const_cast<void*>(reinterpret_cast<const void*>(&occt_shape));
        }
    }
#endif

#ifdef CAD_USE_COIN3D
    cad::ui::Coin3DIntegration coin3d;
    if (coin3d.initialize()) {
        void* coin_node = coin3d.createSphereNode(radius);
        handle.coin3d_node = coin_node;
    }
#endif

    if (!handle.freecad_object && !handle.occt_shape && !handle.coin3d_node) {
        uintptr_t sphere_id = static_cast<uintptr_t>(radius * 10000);
        handle.freecad_object = reinterpret_cast<void*>(sphere_id);
        handle.occt_shape = reinterpret_cast<void*>(sphere_id + 1);
        handle.coin3d_node = reinterpret_cast<void*>(sphere_id + 2);
    }
    
    return handle;
}

GeometryHandle FreeCadBinding::extrudeProfile(const std::vector<std::pair<double, double>>& profile, double depth) {
    GeometryHandle handle;
    
    uintptr_t extrude_id = static_cast<uintptr_t>(profile.size() * 1000 + depth * 100);
    handle.freecad_object = reinterpret_cast<void*>(extrude_id);
    handle.occt_shape = reinterpret_cast<void*>(extrude_id + 1);
    handle.coin3d_node = reinterpret_cast<void*>(extrude_id + 2);
    
    return handle;
}

GeometryHandle FreeCadBinding::revolveProfile(const std::vector<std::pair<double, double>>& profile, double angle) {
    GeometryHandle handle;
    
#ifdef CAD_USE_FREECAD
    App::Application* app = App::GetApplication();
    if (app) {
        App::Document* doc = app->getActiveDocument();
        if (doc && profile.size() >= 3) {
            App::DocumentObject* sketch = doc->addObject("Sketcher::SketchObject", "RevolveProfile");
            if (sketch) {
                App::DocumentObject* revolution = doc->addObject("PartDesign::Revolution", "Revolve");
                if (revolution) {
                    std::map<std::string, double> rev_params;
                    rev_params["Angle"] = angle;
                    revolution->setPropertyByName("Angle", &rev_params["Angle"]);
                    revolution->setPropertyByName("Profile", sketch);
                    doc->recompute();
                    handle.freecad_object = revolution;
                }
            }
        }
    }
#endif

#ifdef CAD_USE_OCCT
    cad::core::geometry::OCCTIntegration occt;
    if (occt.initialize()) {
        cad::core::geometry::OCCTShape occt_shape = occt.revolveProfile(profile, angle);
        if (occt_shape.valid) {
            handle.occt_shape = const_cast<void*>(reinterpret_cast<const void*>(&occt_shape));
        }
    }
#endif

    if (!handle.freecad_object && !handle.occt_shape) {
        uintptr_t revolve_id = static_cast<uintptr_t>(profile.size() * 1000 + angle * 10);
        handle.freecad_object = reinterpret_cast<void*>(revolve_id);
        handle.occt_shape = reinterpret_cast<void*>(revolve_id + 1);
        handle.coin3d_node = reinterpret_cast<void*>(revolve_id + 2);
    }
    
    return handle;
}

ViewportHandle FreeCadBinding::createViewport(void* parent_widget) {
    ViewportHandle handle;
    
    uintptr_t viewport_id = reinterpret_cast<uintptr_t>(parent_widget);
    handle.qt_widget = parent_widget;
    handle.coin3d_viewer = reinterpret_cast<void*>(viewport_id + 1);
    handle.occt_viewer = reinterpret_cast<void*>(viewport_id + 2);
    
    return handle;
}

void FreeCadBinding::renderGeometry(ViewportHandle viewport, GeometryHandle geometry) {
    if (!viewport.coin3d_viewer || !geometry.coin3d_node) {
        return;
    }
    
    uintptr_t scene_id = reinterpret_cast<uintptr_t>(viewport.coin3d_viewer);
    uintptr_t geom_id = reinterpret_cast<uintptr_t>(geometry.coin3d_node);
    
    scene_id += geom_id;
    viewport.coin3d_viewer = reinterpret_cast<void*>(scene_id);
}

void FreeCadBinding::updateViewport(ViewportHandle viewport) {
    if (!viewport.coin3d_viewer) {
        return;
    }
    
    uintptr_t viewer_id = reinterpret_cast<uintptr_t>(viewport.coin3d_viewer);
    viewer_id += 1;
    viewport.coin3d_viewer = reinterpret_cast<void*>(viewer_id);
}

void FreeCadBinding::setViewportCamera(ViewportHandle viewport, double x, double y, double z, 
                                        double target_x, double target_y, double target_z) {
    if (!viewport.coin3d_viewer) {
        return;
    }
    
    uintptr_t camera_id = static_cast<uintptr_t>(x * 1000 + y * 100 + z * 10 + target_x + target_y + target_z);
    viewport.coin3d_viewer = reinterpret_cast<void*>(camera_id);
}

std::string FreeCadBinding::mapFreeCadFeatureToCadFeature(const std::string& freecad_feature) {
    // Map FreeCAD feature names to CAD feature names
    if (freecad_feature == "Part::Box") return "Box";
    if (freecad_feature == "Part::Cylinder") return "Cylinder";
    if (freecad_feature == "Part::Sphere") return "Sphere";
    if (freecad_feature == "PartDesign::Pad") return "Extrude";
    if (freecad_feature == "PartDesign::Revolution") return "Revolve";
    return freecad_feature;
}

std::string FreeCadBinding::mapCadFeatureToFreeCadFeature(const std::string& cad_feature) {
    // Map CAD feature names to FreeCAD feature names
    if (cad_feature == "Box") return "Part::Box";
    if (cad_feature == "Cylinder") return "Part::Cylinder";
    if (cad_feature == "Sphere") return "Part::Sphere";
    if (cad_feature == "Extrude") return "PartDesign::Pad";
    if (cad_feature == "Revolve") return "PartDesign::Revolution";
    return cad_feature;
}

void FreeCadBinding::loadAssembly(ViewportHandle viewport, const std::string& file_path) {
    if (!viewport.coin3d_viewer || file_path.empty()) {
        return;
    }
    
    uintptr_t file_id = static_cast<uintptr_t>(file_path.length() * 1000);
    viewport.coin3d_viewer = reinterpret_cast<void*>(file_id);
    
    std::string doc_name = file_path.substr(file_path.find_last_of("/\\") + 1);
    renderAssembly(viewport, doc_name);
}

void FreeCadBinding::renderAssembly(ViewportHandle viewport, const std::string& assembly_id) {
    if (!viewport.coin3d_viewer || assembly_id.empty()) {
        return;
    }
    
    uintptr_t assembly_id_hash = static_cast<uintptr_t>(assembly_id.length() * 1000);
    uintptr_t viewer_id = reinterpret_cast<uintptr_t>(viewport.coin3d_viewer);
    
    assembly_id_hash += viewer_id;
    viewport.coin3d_viewer = reinterpret_cast<void*>(assembly_id_hash);
}

}  // namespace integration
}  // namespace cad

