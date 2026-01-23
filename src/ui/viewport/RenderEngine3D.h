#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>

namespace cad {
namespace ui {

struct GeometryData {
    std::string id;
    enum Type { Box, Cylinder, Sphere, Extrude, Revolve, Custom } type{Box};
    double params[8]{0.0};
    void* native_handle{nullptr};
};

struct SceneNode {
    std::string id;
    GeometryData geometry;
    double transform[16]{1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
    bool visible{true};
    bool selected{false};
    bool highlighted{false};
    void* render_handle{nullptr};
};

class RenderEngine3D {
public:
    RenderEngine3D();
    ~RenderEngine3D();
    
    bool initialize(int width, int height);
    void shutdown();
    
    std::string createGeometry(const GeometryData& data);
    bool updateGeometry(const std::string& geometry_id, const GeometryData& data);
    bool deleteGeometry(const std::string& geometry_id);
    
    bool addToScene(const std::string& geometry_id, const double* transform = nullptr);
    bool removeFromScene(const std::string& geometry_id);
    void clearScene();
    
    void setCamera(double pos_x, double pos_y, double pos_z,
                   double target_x, double target_y, double target_z,
                   double up_x, double up_y, double up_z,
                   double fov);
    
    void setViewportSize(int width, int height);
    void render();
    
    std::string pickObject(int x, int y) const;
    
    void setDisplayMode(const std::string& geometry_id, int mode);
    void setMaterial(const std::string& geometry_id, float r, float g, float b, float a);
    
    bool isInitialized() const { return initialized_; }
    
    void* getRenderContext() const { return render_context_; }
    
    void enableFrustumCulling(bool enabled) { frustum_culling_enabled_ = enabled; }
    void enableOcclusionCulling(bool enabled) { occlusion_culling_enabled_ = enabled; }
    
private:
    bool initialized_{false};
    void* render_context_{nullptr};
    std::map<std::string, SceneNode> scene_nodes_;
    std::vector<std::string> scene_order_;
    
    double camera_pos_[3]{0, 0, 10};
    double camera_target_[3]{0, 0, 0};
    double camera_up_[3]{0, 1, 0};
    double camera_fov_{45.0};
    int viewport_width_{800};
    int viewport_height_{600};
    bool frustum_culling_enabled_{true};
    bool occlusion_culling_enabled_{false};
    
    bool isInFrustum(const SceneNode& node) const;
    bool isOccluded(const SceneNode& node, const std::vector<SceneNode>& other_nodes) const;
    
    void initializeOpenGL();
    void renderGeometry(const SceneNode& node);
    void applyTransform(const double* transform);
    std::string raycastPick(int x, int y) const;
};

}  // namespace ui
}  // namespace cad

