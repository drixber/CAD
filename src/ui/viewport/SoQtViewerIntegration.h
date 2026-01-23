#pragma once

#ifdef CAD_USE_COIN3D
#ifdef CAD_USE_QT
#include <Inventor/Qt/SoQtExaminerViewer.h>
#include <Inventor/Qt/SoQtRenderArea.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoPerspectiveCamera.h>
#include <Inventor/nodes/SoDirectionalLight.h>
#include <Inventor/nodes/SoMaterial.h>
#include <Inventor/nodes/SoDrawStyle.h>
#include <Inventor/SoDB.h>
#include <QWidget>
#endif
#endif

#include <string>
#include <map>
#include <memory>

namespace cad {
namespace ui {

class SoQtViewerIntegration {
public:
    SoQtViewerIntegration();
    ~SoQtViewerIntegration();
    
    bool initialize(QWidget* parent);
    void shutdown();
    
#ifdef CAD_USE_COIN3D
#ifdef CAD_USE_QT
    SoQtExaminerViewer* getViewer() const { return viewer_; }
    SoSeparator* getSceneRoot() const { return scene_root_; }
    
    void setCamera(double pos_x, double pos_y, double pos_z,
                   double target_x, double target_y, double target_z,
                   double up_x, double up_y, double up_z,
                   double fov);
    
    void addGeometryNode(const std::string& geometry_id, void* coin_node);
    void removeGeometryNode(const std::string& geometry_id);
    void updateGeometryNode(const std::string& geometry_id, void* coin_node);
    
    void setDisplayMode(int mode);
    void setBackgroundColor(float r, float g, float b);
    void showGrid(bool show);
    void showAxes(bool show);
    
    std::string pickObject(int x, int y) const;
    void fitToView();
    void resetCamera();
    
    QWidget* getWidget() const;
#else
    void* getViewer() const { return nullptr; }
    void* getSceneRoot() const { return nullptr; }
    void setCamera(double, double, double, double, double, double, double, double, double, double) {}
    void addGeometryNode(const std::string&, void*) {}
    void removeGeometryNode(const std::string&) {}
    void updateGeometryNode(const std::string&, void*) {}
    void setDisplayMode(int) {}
    void setBackgroundColor(float, float, float) {}
    void showGrid(bool) {}
    void showAxes(bool) {}
    std::string pickObject(int, int) const { return {}; }
    void fitToView() {}
    void resetCamera() {}
    void* getWidget() const { return nullptr; }
#endif
#else
    void* getViewer() const { return nullptr; }
    void* getSceneRoot() const { return nullptr; }
    void setCamera(double, double, double, double, double, double, double, double, double, double) {}
    void addGeometryNode(const std::string&, void*) {}
    void removeGeometryNode(const std::string&) {}
    void updateGeometryNode(const std::string&, void*) {}
    void setDisplayMode(int) {}
    void setBackgroundColor(float, float, float) {}
    void showGrid(bool) {}
    void showAxes(bool) {}
    std::string pickObject(int, int) const { return {}; }
    void fitToView() {}
    void resetCamera() {}
    void* getWidget() const { return nullptr; }
#endif

private:
#ifdef CAD_USE_COIN3D
#ifdef CAD_USE_QT
    SoQtExaminerViewer* viewer_{nullptr};
    SoSeparator* scene_root_{nullptr};
    SoPerspectiveCamera* camera_{nullptr};
    SoDirectionalLight* light_{nullptr};
    SoMaterial* background_material_{nullptr};
    std::map<std::string, SoSeparator*> geometry_nodes_;
    bool initialized_{false};
    
    SoSeparator* grid_node_{nullptr};
    SoSeparator* axes_node_{nullptr};
    bool grid_visible_{false};
    bool axes_visible_{false};
            
            SoSeparator* grid_node_{nullptr};
            SoSeparator* axes_node_{nullptr};
            bool grid_visible_{false};
            bool axes_visible_{false};
#endif
#endif
};

}  // namespace ui
}  // namespace cad
