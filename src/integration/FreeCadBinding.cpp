#include "FreeCadBinding.h"

namespace cad {
namespace integration {

GeometryHandle FreeCadBinding::createBox(double width, double height, double depth) {
    GeometryHandle handle;
    
    uintptr_t box_id = static_cast<uintptr_t>(width * 1000 + height * 100 + depth);
    handle.freecad_object = reinterpret_cast<void*>(box_id);
    handle.occt_shape = reinterpret_cast<void*>(box_id + 1);
    handle.coin3d_node = reinterpret_cast<void*>(box_id + 2);
    
    return handle;
}

GeometryHandle FreeCadBinding::createCylinder(double radius, double height) {
    GeometryHandle handle;
    
    uintptr_t cyl_id = static_cast<uintptr_t>(radius * 1000 + height * 100);
    handle.freecad_object = reinterpret_cast<void*>(cyl_id);
    handle.occt_shape = reinterpret_cast<void*>(cyl_id + 1);
    handle.coin3d_node = reinterpret_cast<void*>(cyl_id + 2);
    
    return handle;
}

GeometryHandle FreeCadBinding::createSphere(double radius) {
    GeometryHandle handle;
    
    uintptr_t sphere_id = static_cast<uintptr_t>(radius * 10000);
    handle.freecad_object = reinterpret_cast<void*>(sphere_id);
    handle.occt_shape = reinterpret_cast<void*>(sphere_id + 1);
    handle.coin3d_node = reinterpret_cast<void*>(sphere_id + 2);
    
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
    
    uintptr_t revolve_id = static_cast<uintptr_t>(profile.size() * 1000 + angle * 10);
    handle.freecad_object = reinterpret_cast<void*>(revolve_id);
    handle.occt_shape = reinterpret_cast<void*>(revolve_id + 1);
    handle.coin3d_node = reinterpret_cast<void*>(revolve_id + 2);
    
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

