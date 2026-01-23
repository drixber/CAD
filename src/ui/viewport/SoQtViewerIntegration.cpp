#include "SoQtViewerIntegration.h"

#ifdef CAD_USE_COIN3D
#ifdef CAD_USE_QT
#include <Inventor/Qt/SoQtExaminerViewer.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoPerspectiveCamera.h>
#include <Inventor/nodes/SoDirectionalLight.h>
#include <Inventor/nodes/SoMaterial.h>
#include <Inventor/nodes/SoDrawStyle.h>
#include <Inventor/nodes/SoCoordinate3.h>
#include <Inventor/nodes/SoLineSet.h>
#include <Inventor/nodes/SoBaseColor.h>
#include <Inventor/SoDB.h>
#include <Inventor/SbVec3f.h>
#include <QWidget>
#endif
#endif

#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace cad {
namespace ui {

SoQtViewerIntegration::SoQtViewerIntegration() = default;

SoQtViewerIntegration::~SoQtViewerIntegration() {
    shutdown();
}

bool SoQtViewerIntegration::initialize(QWidget* parent) {
#ifdef CAD_USE_COIN3D
#ifdef CAD_USE_QT
    if (initialized_) {
        return true;
    }
    
    if (!SoDB::isInitialized()) {
        SoDB::init();
    }
    
    scene_root_ = new SoSeparator;
    scene_root_->ref();
    
    camera_ = new SoPerspectiveCamera;
    camera_->position.setValue(0.0f, 0.0f, 10.0f);
    camera_->pointAt(SbVec3f(0.0f, 0.0f, 0.0f));
    camera_->focalDistance = 10.0f;
    camera_->heightAngle = static_cast<float>(M_PI / 4.0);
    scene_root_->addChild(camera_);
    
    light_ = new SoDirectionalLight;
    light_->direction.setValue(0.0f, 0.0f, -1.0f);
    scene_root_->addChild(light_);
    
    background_material_ = new SoMaterial;
    background_material_->diffuseColor.setValue(0.2f, 0.2f, 0.2f);
    scene_root_->addChild(background_material_);
    
    viewer_ = new SoQtExaminerViewer(parent, nullptr, true, SoQtExaminerViewer::BUILD_POPUP);
    if (!viewer_) {
        scene_root_->unref();
        scene_root_ = nullptr;
        return false;
    }
    
    viewer_->setSceneGraph(scene_root_);
    viewer_->setBackgroundColor(SbColor(0.17f, 0.17f, 0.17f));
    viewer_->setTitle("CAD Viewport");
    viewer_->show();
    
    initialized_ = true;
    return true;
#else
    (void)parent;
    return false;
#endif
#else
    (void)parent;
    return false;
#endif
}

void SoQtViewerIntegration::shutdown() {
#ifdef CAD_USE_COIN3D
#ifdef CAD_USE_QT
    if (viewer_) {
        delete viewer_;
        viewer_ = nullptr;
    }
    
    if (scene_root_) {
        scene_root_->unref();
        scene_root_ = nullptr;
    }
    
            camera_ = nullptr;
            light_ = nullptr;
            background_material_ = nullptr;
            geometry_nodes_.clear();
            
            if (grid_node_) {
                grid_node_->unref();
                grid_node_ = nullptr;
            }
            if (axes_node_) {
                axes_node_->unref();
                axes_node_ = nullptr;
            }
            
            initialized_ = false;
            grid_visible_ = false;
            axes_visible_ = false;
#endif
#endif
}

#ifdef CAD_USE_COIN3D
#ifdef CAD_USE_QT
void SoQtViewerIntegration::setCamera(double pos_x, double pos_y, double pos_z,
                                      double target_x, double target_y, double target_z,
                                      double up_x, double up_y, double up_z,
                                      double fov) {
    if (!camera_) {
        return;
    }
    
    camera_->position.setValue(static_cast<float>(pos_x), static_cast<float>(pos_y), static_cast<float>(pos_z));
    camera_->pointAt(SbVec3f(static_cast<float>(target_x), static_cast<float>(target_y), static_cast<float>(target_z)));
    camera_->focalDistance = static_cast<float>(std::sqrt(
        (pos_x - target_x) * (pos_x - target_x) +
        (pos_y - target_y) * (pos_y - target_y) +
        (pos_z - target_z) * (pos_z - target_z)
    ));
    camera_->heightAngle = static_cast<float>(fov * M_PI / 180.0);
    
    if (viewer_) {
        viewer_->viewAll();
    }
}

void SoQtViewerIntegration::addGeometryNode(const std::string& geometry_id, void* coin_node) {
    if (!scene_root_ || !coin_node) {
        return;
    }
    
    SoSeparator* node = static_cast<SoSeparator*>(coin_node);
    if (geometry_nodes_.find(geometry_id) == geometry_nodes_.end()) {
        scene_root_->addChild(node);
        geometry_nodes_[geometry_id] = node;
    }
}

void SoQtViewerIntegration::removeGeometryNode(const std::string& geometry_id) {
    if (!scene_root_) {
        return;
    }
    
    auto it = geometry_nodes_.find(geometry_id);
    if (it != geometry_nodes_.end()) {
        scene_root_->removeChild(it->second);
        geometry_nodes_.erase(it);
    }
}

void SoQtViewerIntegration::updateGeometryNode(const std::string& geometry_id, void* coin_node) {
    removeGeometryNode(geometry_id);
    addGeometryNode(geometry_id, coin_node);
}

void SoQtViewerIntegration::setDisplayMode(int mode) {
    if (!scene_root_) {
        return;
    }
    
    for (auto& pair : geometry_nodes_) {
        SoSeparator* node = pair.second;
        if (node) {
            SoDrawStyle* style = new SoDrawStyle;
            if (mode == 0) {
                style->style = SoDrawStyle::LINES;
            } else if (mode == 1) {
                style->style = SoDrawStyle::FILLED;
            } else {
                style->style = SoDrawStyle::FILLED;
            }
            node->addChild(style);
        }
    }
}

void SoQtViewerIntegration::setBackgroundColor(float r, float g, float b) {
    if (viewer_) {
        viewer_->setBackgroundColor(SbColor(r, g, b));
    }
}

void SoQtViewerIntegration::showGrid(bool show) {
    if (!scene_root_) {
        return;
    }
    
    grid_visible_ = show;
    
    if (show && !grid_node_) {
        grid_node_ = new SoSeparator;
        grid_node_->ref();
        
        SoBaseColor* grid_color = new SoBaseColor;
        grid_color->rgb.setValue(0.5f, 0.5f, 0.5f);
        grid_node_->addChild(grid_color);
        
        SoCoordinate3* grid_coords = new SoCoordinate3;
        SoLineSet* grid_lines = new SoLineSet;
        
        const int grid_size = 20;
        const float grid_spacing = 1.0f;
        const float grid_extent = grid_size * grid_spacing * 0.5f;
        
        std::vector<SbVec3f> coords;
        std::vector<int32_t> line_counts;
        
        for (int i = -grid_size / 2; i <= grid_size / 2; ++i) {
            float pos = i * grid_spacing;
            coords.push_back(SbVec3f(-grid_extent, pos, 0.0f));
            coords.push_back(SbVec3f(grid_extent, pos, 0.0f));
            line_counts.push_back(2);
            
            coords.push_back(SbVec3f(pos, -grid_extent, 0.0f));
            coords.push_back(SbVec3f(pos, grid_extent, 0.0f));
            line_counts.push_back(2);
        }
        
        grid_coords->point.setValues(0, static_cast<int>(coords.size()), coords.data());
        grid_lines->numVertices.setValues(0, static_cast<int>(line_counts.size()), line_counts.data());
        
        grid_node_->addChild(grid_coords);
        grid_node_->addChild(grid_lines);
        
        scene_root_->addChild(grid_node_);
    } else if (!show && grid_node_) {
        scene_root_->removeChild(grid_node_);
        grid_node_->unref();
        grid_node_ = nullptr;
    }
    
    if (viewer_) {
        viewer_->viewAll();
    }
}

void SoQtViewerIntegration::showAxes(bool show) {
    if (!scene_root_) {
        return;
    }
    
    axes_visible_ = show;
    
    if (show && !axes_node_) {
        axes_node_ = new SoSeparator;
        axes_node_->ref();
        
        const float axis_length = 5.0f;
        const float axis_radius = 0.05f;
        
        SoCoordinate3* axis_coords = new SoCoordinate3;
        SoLineSet* axis_lines = new SoLineSet;
        
        std::vector<SbVec3f> coords;
        std::vector<int32_t> line_counts;
        
        coords.push_back(SbVec3f(0.0f, 0.0f, 0.0f));
        coords.push_back(SbVec3f(axis_length, 0.0f, 0.0f));
        line_counts.push_back(2);
        
        coords.push_back(SbVec3f(0.0f, 0.0f, 0.0f));
        coords.push_back(SbVec3f(0.0f, axis_length, 0.0f));
        line_counts.push_back(2);
        
        coords.push_back(SbVec3f(0.0f, 0.0f, 0.0f));
        coords.push_back(SbVec3f(0.0f, 0.0f, axis_length));
        line_counts.push_back(2);
        
        axis_coords->point.setValues(0, static_cast<int>(coords.size()), coords.data());
        axis_lines->numVertices.setValues(0, static_cast<int>(line_counts.size()), line_counts.data());
        
        SoBaseColor* x_color = new SoBaseColor;
        x_color->rgb.setValue(1.0f, 0.0f, 0.0f);
        axes_node_->addChild(x_color);
        axes_node_->addChild(axis_coords);
        axes_node_->addChild(axis_lines);
        
        SoBaseColor* y_color = new SoBaseColor;
        y_color->rgb.setValue(0.0f, 1.0f, 0.0f);
        axes_node_->addChild(y_color);
        
        SoBaseColor* z_color = new SoBaseColor;
        z_color->rgb.setValue(0.0f, 0.0f, 1.0f);
        axes_node_->addChild(z_color);
        
        scene_root_->addChild(axes_node_);
    } else if (!show && axes_node_) {
        scene_root_->removeChild(axes_node_);
        axes_node_->unref();
        axes_node_ = nullptr;
    }
    
    if (viewer_) {
        viewer_->viewAll();
    }
}

std::string SoQtViewerIntegration::pickObject(int x, int y) const {
    if (!viewer_ || !scene_root_) {
        return {};
    }
    
    SoRayPickAction pick_action(viewer_->getViewportRegion());
    pick_action.setPoint(SbVec2s(static_cast<short>(x), static_cast<short>(y)));
    pick_action.setRadius(5.0f);
    pick_action.apply(scene_root_);
    
    const SoPickedPoint* picked = pick_action.getPickedPoint();
    if (!picked) {
        return {};
    }
    
    SoPath* path = picked->getPath();
    if (!path) {
        return {};
    }
    
    for (int i = path->getLength() - 1; i >= 0; --i) {
        SoNode* node = path->getNode(i);
        if (!node) {
            continue;
        }
        
        for (const auto& pair : geometry_nodes_) {
            if (pair.second == node) {
                return pair.first;
            }
        }
    }
    
    return {};
}

void SoQtViewerIntegration::fitToView() {
    if (viewer_) {
        viewer_->viewAll();
    }
}

void SoQtViewerIntegration::resetCamera() {
    if (camera_) {
        camera_->position.setValue(0.0f, 0.0f, 10.0f);
        camera_->pointAt(SbVec3f(0.0f, 0.0f, 0.0f));
        camera_->focalDistance = 10.0f;
    }
    if (viewer_) {
        viewer_->viewAll();
    }
}

QWidget* SoQtViewerIntegration::getWidget() const {
    if (viewer_) {
        return viewer_->getWidget();
    }
    return nullptr;
}
#endif
#endif

}  // namespace ui
}  // namespace cad
