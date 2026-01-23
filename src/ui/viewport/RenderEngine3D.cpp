#include "RenderEngine3D.h"
#include "Coin3DIntegration.h"

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
    
#ifdef CAD_USE_COIN3D
    coin3d_integration_ = std::make_unique<Coin3DIntegration>();
    if (!coin3d_integration_->initialize()) {
        return false;
    }
#else
    initializeOpenGL();
#endif
    
    initialized_ = true;
    return true;
}

void RenderEngine3D::shutdown() {
    clearScene();
#ifdef CAD_USE_COIN3D
    if (coin3d_integration_) {
        coin3d_integration_->shutdown();
        coin3d_integration_.reset();
    }
#endif
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
    
#ifdef CAD_USE_COIN3D
    if (coin3d_integration_) {
        void* coin_node = coin3d_integration_->createGeometryNode(data.id, const_cast<GeometryData*>(&data));
        node.render_handle = coin_node;
    } else {
        node.render_handle = reinterpret_cast<void*>(static_cast<uintptr_t>(scene_nodes_.size() + 1));
    }
#else
    node.render_handle = reinterpret_cast<void*>(static_cast<uintptr_t>(scene_nodes_.size() + 1));
#endif
    
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
    
#ifdef CAD_USE_COIN3D
    if (coin3d_integration_ && coin3d_integration_->getSceneRoot()) {
        for (const auto& geometry_id : scene_order_) {
            auto it = scene_nodes_.find(geometry_id);
            if (it != scene_nodes_.end() && it->second.visible) {
                if (frustum_culling_enabled_ && !isInFrustum(it->second)) {
                    continue;
                }
                
                void* coin_node = it->second.render_handle;
                if (coin_node) {
                    coin3d_integration_->addToScene(
                        static_cast<void*>(coin3d_integration_->getSceneRoot()),
                        coin_node
                    );
                }
            }
        }
        return;
    }
#endif
    
    std::vector<SceneNode> visible_nodes;
    visible_nodes.reserve(scene_order_.size());
    
    for (const auto& geometry_id : scene_order_) {
        auto it = scene_nodes_.find(geometry_id);
        if (it != scene_nodes_.end() && it->second.visible) {
            if (frustum_culling_enabled_ && !isInFrustum(it->second)) {
                continue;
            }
            visible_nodes.push_back(it->second);
        }
    }
    
    if (occlusion_culling_enabled_) {
        std::vector<SceneNode> culled_nodes;
        for (const auto& node : visible_nodes) {
            if (!isOccluded(node, visible_nodes)) {
                culled_nodes.push_back(node);
            }
        }
        visible_nodes = culled_nodes;
    }
    
    for (const auto& node : visible_nodes) {
        renderGeometry(node);
    }
}

std::string RenderEngine3D::pickObject(int x, int y) const {
#ifdef CAD_USE_COIN3D
    if (coin3d_integration_ && coin3d_integration_->getSceneRoot()) {
        return coin3d_integration_->pickObject(
            coin3d_integration_->getSceneRoot(),
            x, y,
            viewport_width_, viewport_height_,
            camera_pos_, camera_target_, camera_up_,
            camera_fov_
        );
    }
#endif
    return raycastPick(x, y);
}

void RenderEngine3D::setDisplayMode(const std::string& geometry_id, int mode) {
    auto it = scene_nodes_.find(geometry_id);
    if (it != scene_nodes_.end()) {
        it->second.geometry.params[7] = static_cast<double>(mode);
#ifdef CAD_USE_COIN3D
        if (coin3d_integration_ && it->second.render_handle) {
            coin3d_integration_->setDisplayMode(it->second.render_handle, mode);
        }
#endif
    }
}

void RenderEngine3D::setMaterial(const std::string& geometry_id, float r, float g, float b, float a) {
    auto it = scene_nodes_.find(geometry_id);
    if (it != scene_nodes_.end()) {
        it->second.geometry.params[0] = r;
        it->second.geometry.params[1] = g;
        it->second.geometry.params[2] = b;
        it->second.geometry.params[3] = a;
#ifdef CAD_USE_COIN3D
        if (coin3d_integration_ && it->second.render_handle) {
            coin3d_integration_->setMaterial(it->second.render_handle, r, g, b, a);
        }
#endif
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

bool RenderEngine3D::isInFrustum(const SceneNode& node) const {
    double node_pos[3] = {node.transform[12], node.transform[13], node.transform[14]};
    
    double dx = node_pos[0] - camera_pos_[0];
    double dy = node_pos[1] - camera_pos_[1];
    double dz = node_pos[2] - camera_pos_[2];
    double dist = std::sqrt(dx*dx + dy*dy + dz*dz);
    
    double view_dir[3] = {
        camera_target_[0] - camera_pos_[0],
        camera_target_[1] - camera_pos_[1],
        camera_target_[2] - camera_pos_[2]
    };
    double view_dist = std::sqrt(view_dir[0]*view_dir[0] + view_dir[1]*view_dir[1] + view_dir[2]*view_dir[2]);
    
    if (view_dist < 0.001) {
        return true;
    }
    
    double dot = (dx*view_dir[0] + dy*view_dir[1] + dz*view_dir[2]) / view_dist;
    
    double fov_rad = camera_fov_ * M_PI / 180.0;
    double near_plane = 0.1;
    double far_plane = 1000.0;
    
    if (dot < near_plane || dot > far_plane) {
        return false;
    }
    
    double aspect = static_cast<double>(viewport_width_) / viewport_height_;
    double tan_fov = std::tan(fov_rad * 0.5);
    double max_side_dist = dot * tan_fov * aspect;
    
    double side_dist = std::sqrt((dx*dx + dy*dy + dz*dz) - dot*dot);
    
    return side_dist < max_side_dist * 1.5;
}

bool RenderEngine3D::isOccluded(const SceneNode& node, const std::vector<SceneNode>& other_nodes) const {
    double node_pos[3] = {node.transform[12], node.transform[13], node.transform[14]};
    
    double view_dir[3] = {
        camera_target_[0] - camera_pos_[0],
        camera_target_[1] - camera_pos_[1],
        camera_target_[2] - camera_pos_[2]
    };
    double view_dist = std::sqrt(view_dir[0]*view_dir[0] + view_dir[1]*view_dir[1] + view_dir[2]*view_dir[2]);
    
    if (view_dist < 0.001) {
        return false;
    }
    
    double node_dist = std::sqrt(
        (node_pos[0] - camera_pos_[0]) * (node_pos[0] - camera_pos_[0]) +
        (node_pos[1] - camera_pos_[1]) * (node_pos[1] - camera_pos_[1]) +
        (node_pos[2] - camera_pos_[2]) * (node_pos[2] - camera_pos_[2])
    );
    
    for (const auto& other : other_nodes) {
        if (other.id == node.id) {
            continue;
        }
        
        double other_pos[3] = {other.transform[12], other.transform[13], other.transform[14]};
        double other_dist = std::sqrt(
            (other_pos[0] - camera_pos_[0]) * (other_pos[0] - camera_pos_[0]) +
            (other_pos[1] - camera_pos_[1]) * (other_pos[1] - camera_pos_[1]) +
            (other_pos[2] - camera_pos_[2]) * (other_pos[2] - camera_pos_[2])
        );
        
        if (other_dist < node_dist) {
            double dx = other_pos[0] - node_pos[0];
            double dy = other_pos[1] - node_pos[1];
            double dz = other_pos[2] - node_pos[2];
            double dist_between = std::sqrt(dx*dx + dy*dy + dz*dz);
            
            if (dist_between < 10.0) {
                return true;
            }
        }
    }
    
    return false;
}

}  // namespace ui
}  // namespace cad

