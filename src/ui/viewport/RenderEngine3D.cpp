#include "RenderEngine3D.h"

#include <cmath>
#include <algorithm>
#include <limits>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace cad {
namespace ui {

RenderEngine3D::RenderEngine3D() = default;

RenderEngine3D::~RenderEngine3D() {
    shutdown();
}

bool RenderEngine3D::initialize(int width, int height) {
    if (initialized_) {
        return true;
    }
    
    viewport_width_ = width;
    viewport_height_ = height;
    
    initializeOpenGL();
    
    initialized_ = true;
    return true;
}

void RenderEngine3D::shutdown() {
    clearScene();
    initialized_ = false;
}

void RenderEngine3D::initializeOpenGL() {
    render_context_ = reinterpret_cast<void*>(0x12345678);
}

std::string RenderEngine3D::createGeometry(const GeometryData& data) {
    if (data.id.empty()) {
        return {};
    }
    
    SceneNode node;
    node.id = data.id;
    node.geometry = data;
    node.render_handle = reinterpret_cast<void*>(static_cast<uintptr_t>(scene_nodes_.size() + 1));
    
    scene_nodes_[data.id] = node;
    scene_order_.push_back(data.id);
    
    return data.id;
}

bool RenderEngine3D::updateGeometry(const std::string& geometry_id, const GeometryData& data) {
    auto it = scene_nodes_.find(geometry_id);
    if (it == scene_nodes_.end()) {
        return false;
    }
    
    it->second.geometry = data;
    return true;
}

bool RenderEngine3D::deleteGeometry(const std::string& geometry_id) {
    auto it = scene_nodes_.find(geometry_id);
    if (it == scene_nodes_.end()) {
        return false;
    }
    
    removeFromScene(geometry_id);
    scene_nodes_.erase(it);
    scene_order_.erase(
        std::remove(scene_order_.begin(), scene_order_.end(), geometry_id),
        scene_order_.end()
    );
    
    return true;
}

bool RenderEngine3D::addToScene(const std::string& geometry_id, const double* transform) {
    auto it = scene_nodes_.find(geometry_id);
    if (it == scene_nodes_.end()) {
        return false;
    }
    
    it->second.visible = true;
    if (transform) {
        std::copy(transform, transform + 16, it->second.transform);
    }
    
    return true;
}

bool RenderEngine3D::removeFromScene(const std::string& geometry_id) {
    auto it = scene_nodes_.find(geometry_id);
    if (it == scene_nodes_.end()) {
        return false;
    }
    
    it->second.visible = false;
    return true;
}

void RenderEngine3D::clearScene() {
    for (auto& [id, node] : scene_nodes_) {
        node.visible = false;
    }
    scene_order_.clear();
}

void RenderEngine3D::setCamera(double pos_x, double pos_y, double pos_z,
                                double target_x, double target_y, double target_z,
                                double up_x, double up_y, double up_z,
                                double fov) {
    camera_pos_[0] = pos_x;
    camera_pos_[1] = pos_y;
    camera_pos_[2] = pos_z;
    camera_target_[0] = target_x;
    camera_target_[1] = target_y;
    camera_target_[2] = target_z;
    camera_up_[0] = up_x;
    camera_up_[1] = up_y;
    camera_up_[2] = up_z;
    camera_fov_ = fov;
}

void RenderEngine3D::setViewportSize(int width, int height) {
    viewport_width_ = width;
    viewport_height_ = height;
}

void RenderEngine3D::render() {
    if (!initialized_) {
        return;
    }
    
    for (const auto& geometry_id : scene_order_) {
        auto it = scene_nodes_.find(geometry_id);
        if (it != scene_nodes_.end() && it->second.visible) {
            renderGeometry(it->second);
        }
    }
}

std::string RenderEngine3D::pickObject(int x, int y) const {
    return raycastPick(x, y);
}

void RenderEngine3D::setDisplayMode(const std::string& geometry_id, int mode) {
    auto it = scene_nodes_.find(geometry_id);
    if (it != scene_nodes_.end()) {
        it->second.geometry.params[7] = static_cast<double>(mode);
    }
}

void RenderEngine3D::setMaterial(const std::string& geometry_id, float r, float g, float b, float a) {
    auto it = scene_nodes_.find(geometry_id);
    if (it != scene_nodes_.end()) {
        it->second.geometry.params[0] = r;
        it->second.geometry.params[1] = g;
        it->second.geometry.params[2] = b;
        it->second.geometry.params[3] = a;
    }
}

void RenderEngine3D::renderGeometry(const SceneNode& node) {
    applyTransform(node.transform);
    
    const GeometryData& geom = node.geometry;
    
    switch (geom.type) {
        case GeometryData::Box:
            break;
        case GeometryData::Cylinder:
            break;
        case GeometryData::Sphere:
            break;
        case GeometryData::Extrude:
            break;
        case GeometryData::Revolve:
            break;
        case GeometryData::Custom:
            break;
    }
}

void RenderEngine3D::applyTransform(const double* transform) {
    (void)transform;
}

std::string RenderEngine3D::raycastPick(int x, int y) const {
    double normalized_x = (2.0 * x / viewport_width_) - 1.0;
    double normalized_y = 1.0 - (2.0 * y / viewport_height_);
    
    double ray_dir[3];
    double ray_origin[3] = {camera_pos_[0], camera_pos_[1], camera_pos_[2]};
    
    double dx = camera_target_[0] - camera_pos_[0];
    double dy = camera_target_[1] - camera_pos_[1];
    double dz = camera_target_[2] - camera_pos_[2];
    double dist = std::sqrt(dx*dx + dy*dy + dz*dz);
    
    if (dist < 0.001) {
        return {};
    }
    
    double aspect = static_cast<double>(viewport_width_) / viewport_height_;
    double fov_rad = camera_fov_ * M_PI / 180.0;
    double tan_fov = std::tan(fov_rad * 0.5);
    
    ray_dir[0] = dx / dist + normalized_x * tan_fov * aspect;
    ray_dir[1] = dy / dist + normalized_y * tan_fov;
    ray_dir[2] = dz / dist;
    
    double min_dist = std::numeric_limits<double>::max();
    std::string closest_id;
    
    for (const auto& [id, node] : scene_nodes_) {
        if (!node.visible) {
            continue;
        }
        
        double node_pos[3] = {node.transform[12], node.transform[13], node.transform[14]};
        
        double to_node[3] = {
            node_pos[0] - ray_origin[0],
            node_pos[1] - ray_origin[1],
            node_pos[2] - ray_origin[2]
        };
        
        double dot = ray_dir[0] * to_node[0] + ray_dir[1] * to_node[1] + ray_dir[2] * to_node[2];
        if (dot < 0) {
            continue;
        }
        
        double dist_to_node = std::sqrt(to_node[0]*to_node[0] + to_node[1]*to_node[1] + to_node[2]*to_node[2]);
        double proj_dist = dot;
        
        if (proj_dist < min_dist && proj_dist < dist_to_node + 1.0) {
            min_dist = proj_dist;
            closest_id = id;
        }
    }
    
    return closest_id;
}

}  // namespace ui
}  // namespace cad

