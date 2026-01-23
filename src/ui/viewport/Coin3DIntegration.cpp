#include "Coin3DIntegration.h"

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
#include <Inventor/actions/SoRayPickAction.h>
#include <Inventor/actions/SoGetBoundingBoxAction.h>
#include <Inventor/SoPickedPoint.h>
#include <Inventor/SoPath.h>
#include <Inventor/SoLists.h>
#endif

#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace cad {
namespace ui {

Coin3DIntegration::Coin3DIntegration()
#ifdef CAD_USE_COIN3D
    : scene_root_(nullptr)
#endif
{
}

Coin3DIntegration::~Coin3DIntegration() {
    shutdown();
}

bool Coin3DIntegration::initialize() {
#ifdef CAD_USE_COIN3D
    if (initialized_) {
        return true;
    }
    
    SoDB::init();
    
    scene_root_ = new SoSeparator;
    scene_root_->ref();
    
    initialized_ = true;
    return true;
#else
    initialized_ = true;
    return true;
#endif
}

void Coin3DIntegration::shutdown() {
#ifdef CAD_USE_COIN3D
    if (scene_root_) {
        scene_root_->unref();
        scene_root_ = nullptr;
    }
    
    geometry_nodes_.clear();
#endif
    initialized_ = false;
}

#ifdef CAD_USE_COIN3D
SoSeparator* Coin3DIntegration::createBoxNode(double width, double height, double depth) {
    SoSeparator* separator = new SoSeparator;
    
    SoTransform* transform = new SoTransform;
    transform->scaleFactor.setValue(width, height, depth);
    separator->addChild(transform);
    
    SoCube* cube = new SoCube;
    cube->width = 1.0;
    cube->height = 1.0;
    cube->depth = 1.0;
    separator->addChild(cube);
    
    return separator;
}

SoSeparator* Coin3DIntegration::createCylinderNode(double radius, double height) {
    SoSeparator* separator = new SoSeparator;
    
    SoTransform* transform = new SoTransform;
    transform->scaleFactor.setValue(radius * 2.0, height, radius * 2.0);
    separator->addChild(transform);
    
    SoCylinder* cylinder = new SoCylinder;
    cylinder->radius = 0.5;
    cylinder->height = 1.0;
    separator->addChild(cylinder);
    
    return separator;
}

SoSeparator* Coin3DIntegration::createSphereNode(double radius) {
    SoSeparator* separator = new SoSeparator;
    
    SoTransform* transform = new SoTransform;
    transform->scaleFactor.setValue(radius * 2.0, radius * 2.0, radius * 2.0);
    separator->addChild(transform);
    
    SoSphere* sphere = new SoSphere;
    sphere->radius = 0.5;
    separator->addChild(sphere);
    
    return separator;
}

SoSeparator* Coin3DIntegration::createGeometryNode(const std::string& geometry_id, void* geometry_data) {
    if (geometry_nodes_.find(geometry_id) != geometry_nodes_.end()) {
        return geometry_nodes_[geometry_id];
    }
    
    SoSeparator* node = nullptr;
    
    if (geometry_data) {
        GeometryData* data = static_cast<GeometryData*>(geometry_data);
        switch (data->type) {
            case GeometryData::Box:
                node = createBoxNode(data->params[0], data->params[1], data->params[2]);
                break;
            case GeometryData::Cylinder:
                node = createCylinderNode(data->params[0], data->params[1]);
                break;
            case GeometryData::Sphere:
                node = createSphereNode(data->params[0]);
                break;
            default:
                node = new SoSeparator;
                break;
        }
    } else {
        node = new SoSeparator;
    }
    
    if (node) {
        node->ref();
        geometry_nodes_[geometry_id] = node;
    }
    
    return node;
}

void Coin3DIntegration::addToScene(SoSeparator* root, SoSeparator* node) {
    if (root && node) {
        root->addChild(node);
    }
}

void Coin3DIntegration::removeFromScene(SoSeparator* root, SoSeparator* node) {
    if (root && node) {
        root->removeChild(node);
    }
}

void Coin3DIntegration::updateTransform(SoSeparator* node, const double* transform) {
    if (!node || !transform) {
        return;
    }
    
    SoTransform* so_transform = nullptr;
    for (int i = 0; i < node->getNumChildren(); ++i) {
        SoNode* child = node->getChild(i);
        if (child->isOfType(SoTransform::getClassTypeId())) {
            so_transform = static_cast<SoTransform*>(child);
            break;
        }
    }
    
    if (!so_transform) {
        so_transform = new SoTransform;
        node->insertChild(so_transform, 0);
    }
    
    SbVec3f translation(transform[12], transform[13], transform[14]);
    so_transform->translation = translation;
    
    SbMatrix matrix;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            matrix[i][j] = transform[i * 4 + j];
        }
    }
    SbRotation rotation;
    SbVec3f scale;
    SbVec3f scale_orientation;
    SbVec3f center;
    matrix.getTransform(translation, rotation, scale, scale_orientation, center);
    so_transform->rotation = rotation;
    so_transform->scaleFactor = scale;
}

void Coin3DIntegration::setMaterial(SoSeparator* node, float r, float g, float b, float a) {
    if (!node) {
        return;
    }
    
    SoMaterial* material = nullptr;
    for (int i = 0; i < node->getNumChildren(); ++i) {
        SoNode* child = node->getChild(i);
        if (child->isOfType(SoMaterial::getClassTypeId())) {
            material = static_cast<SoMaterial*>(child);
            break;
        }
    }
    
    if (!material) {
        material = new SoMaterial;
        node->insertChild(material, 0);
    }
    
    material->diffuseColor.setValue(r, g, b);
    material->transparency = 1.0f - a;
}

void Coin3DIntegration::setDisplayMode(SoSeparator* node, int mode) {
    if (!node) {
        return;
    }
    
    SoDrawStyle* style = nullptr;
    for (int i = 0; i < node->getNumChildren(); ++i) {
        SoNode* child = node->getChild(i);
        if (child->isOfType(SoDrawStyle::getClassTypeId())) {
            style = static_cast<SoDrawStyle*>(child);
            break;
        }
    }
    
    if (!style) {
        style = new SoDrawStyle;
        node->insertChild(style, 0);
    }
    
    switch (mode) {
        case 0:
            style->style = SoDrawStyle::FILLED;
            break;
        case 1:
            style->style = SoDrawStyle::LINES;
            break;
        case 2:
            style->style = SoDrawStyle::POINTS;
            break;
        default:
            style->style = SoDrawStyle::FILLED;
            break;
    }
}

std::string Coin3DIntegration::pickObject(SoSeparator* scene_root, int x, int y, int viewport_width, int viewport_height,
                                          const double* camera_pos, const double* camera_target, const double* camera_up, double fov) {
    if (!scene_root) {
        return {};
    }
    
    SoRayPickAction pick_action(SbViewportRegion(viewport_width, viewport_height));
    
    double normalized_x = (2.0 * x / viewport_width) - 1.0;
    double normalized_y = 1.0 - (2.0 * y / viewport_height);
    
    SbVec3f ray_origin(camera_pos[0], camera_pos[1], camera_pos[2]);
    SbVec3f view_dir(camera_target[0] - camera_pos[0],
                     camera_target[1] - camera_pos[1],
                     camera_target[2] - camera_pos[2]);
    view_dir.normalize();
    
    double fov_rad = fov * M_PI / 180.0;
    double tan_fov = std::tan(fov_rad * 0.5);
    double aspect = static_cast<double>(viewport_width) / viewport_height;
    
    SbVec3f right = view_dir.cross(SbVec3f(camera_up[0], camera_up[1], camera_up[2]));
    right.normalize();
    SbVec3f up = right.cross(view_dir);
    up.normalize();
    
    SbVec3f ray_dir = view_dir + right * (normalized_x * tan_fov * aspect) + up * (normalized_y * tan_fov);
    ray_dir.normalize();
    
    pick_action.setRay(ray_origin, ray_dir);
    pick_action.apply(scene_root);
    
    const SoPickedPoint* picked = pick_action.getPickedPoint();
    if (picked) {
        const SoPath* path = picked->getPath();
        if (path && path->getLength() > 0) {
            SoNode* node = path->getNode(path->getLength() - 1);
            if (node) {
                for (const auto& [id, geom_node] : geometry_nodes_) {
                    if (geom_node == node || path->containsNode(geom_node)) {
                        return id;
                    }
                }
            }
        }
    }
    
    return {};
}
#endif

}  // namespace ui
}  // namespace cad
