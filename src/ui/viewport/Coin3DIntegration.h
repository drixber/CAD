#pragma once

#ifdef CAD_USE_COIN3D
#include <Inventor/SbVec3f.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoCube.h>
#include <Inventor/nodes/SoCylinder.h>
#include <Inventor/nodes/SoSphere.h>
#include <Inventor/nodes/SoTransform.h>
#include <Inventor/nodes/SoMaterial.h>
#include <Inventor/nodes/SoDrawStyle.h>
#include <Inventor/nodes/SoComplexity.h>
#include <Inventor/SoDB.h>
#include <Inventor/SoInput.h>
#include <Inventor/SoOutput.h>
#include <Inventor/actions/SoRayPickAction.h>
#include <Inventor/actions/SoGetBoundingBoxAction.h>
#include <Inventor/events/SoMouseButtonEvent.h>
#include <Inventor/events/SoLocation2Event.h>
#include <Inventor/SoPickedPoint.h>
#include <Inventor/SoPath.h>
#include <Inventor/SoLists.h>
#include <Inventor/SoPath.h>
#include <Inventor/SoLists.h>
#endif

#include <string>
#include <map>
#include <memory>

namespace cad {
namespace ui {

class Coin3DIntegration {
public:
    Coin3DIntegration();
    ~Coin3DIntegration();
    
    bool initialize();
    void shutdown();
    
#ifdef CAD_USE_COIN3D
    SoSeparator* createBoxNode(double width, double height, double depth);
    SoSeparator* createCylinderNode(double radius, double height);
    SoSeparator* createSphereNode(double radius);
    SoSeparator* createGeometryNode(const std::string& geometry_id, void* geometry_data);
    
    void addToScene(SoSeparator* root, SoSeparator* node);
    void removeFromScene(SoSeparator* root, SoSeparator* node);
    void updateTransform(SoSeparator* node, const double* transform);
    void setMaterial(SoSeparator* node, float r, float g, float b, float a);
    void setDisplayMode(SoSeparator* node, int mode);
    
    std::string pickObject(SoSeparator* scene_root, int x, int y, int viewport_width, int viewport_height,
                          const double* camera_pos, const double* camera_target, const double* camera_up, double fov);
    
    SoSeparator* getSceneRoot() const { return scene_root_; }
#else
    void* createBoxNode(double width, double height, double depth) { return nullptr; }
    void* createCylinderNode(double radius, double height) { return nullptr; }
    void* createSphereNode(double radius) { return nullptr; }
    void* createGeometryNode(const std::string& geometry_id, void* geometry_data) { return nullptr; }
    void addToScene(void* root, void* node) {}
    void removeFromScene(void* root, void* node) {}
    void updateTransform(void* node, const double* transform) {}
    void setMaterial(void* node, float r, float g, float b, float a) {}
    void setDisplayMode(void* node, int mode) {}
    std::string pickObject(void* scene_root, int x, int y, int viewport_width, int viewport_height,
                          const double* camera_pos, const double* camera_target, const double* camera_up, double fov) { return {}; }
    void* getSceneRoot() const { return nullptr; }
#endif

private:
#ifdef CAD_USE_COIN3D
    SoSeparator* scene_root_;
    std::map<std::string, SoSeparator*> geometry_nodes_;
#endif
    bool initialized_{false};
};

}  // namespace ui
}  // namespace cad
