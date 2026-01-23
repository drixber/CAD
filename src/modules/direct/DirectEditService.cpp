#include "DirectEditService.h"

#include <algorithm>
#include <cmath>
#include <vector>

namespace cad {
namespace modules {

DirectEditResult DirectEditService::applyEdit(const DirectEditRequest& request) const {
    DirectEditResult result;
    
    if (request.targetFeature.empty()) {
        result.success = false;
        result.message = "No target feature specified";
        return result;
    }
    
    if (request.selected_faces.empty()) {
        result.success = false;
        result.message = "No faces selected";
        return result;
    }
    
    if (!validateEdit(request)) {
        result.success = false;
        result.message = "Edit validation failed";
        return result;
    }
    
    if (request.preview_mode) {
        return previewEdit(request);
    }
    
    switch (request.operation) {
        case DirectEditOperation::MoveFace:
            return moveFace(request);
        case DirectEditOperation::OffsetFace:
            return offsetFace(request);
        case DirectEditOperation::DeleteFace:
            return deleteFace(request);
        case DirectEditOperation::Freeform:
            return freeformEdit(request);
        default:
            result.success = false;
            result.message = "Unknown operation";
            return result;
    }
}

DirectEditResult DirectEditService::moveFace(const DirectEditRequest& request) const {
    DirectEditResult result;
    result.success = true;
    result.message = "Face moved successfully";
    result.modified_feature_id = request.targetFeature + "_moved";
    
    // Calculate volume and surface area changes
    result.volume_change = calculateVolumeChange(request);
    result.surface_area_change = calculateSurfaceAreaChange(request);
    
    result.modified_faces = request.selected_faces;
    for (auto& face : result.modified_faces) {
        if (request.move_params.direction.size() >= 3) {
            double dx = request.move_params.direction[0] * request.move_params.distance;
            double dy = request.move_params.direction[1] * request.move_params.distance;
            double dz = request.move_params.direction[2] * request.move_params.distance;
            
            if (face.normal.size() >= 3) {
                face.normal[0] += dx * 0.1;
                face.normal[1] += dy * 0.1;
                face.normal[2] += dz * 0.1;
                
                double len = std::sqrt(face.normal[0]*face.normal[0] + 
                                      face.normal[1]*face.normal[1] + 
                                      face.normal[2]*face.normal[2]);
                if (len > 0.001) {
                    face.normal[0] /= len;
                    face.normal[1] /= len;
                    face.normal[2] /= len;
                }
            }
        }
    }
    
    const_cast<DirectEditService*>(this)->updateFeatureHistory(request.targetFeature, result);
    
    return result;
}

DirectEditResult DirectEditService::offsetFace(const DirectEditRequest& request) const {
    DirectEditResult result;
    result.success = true;
    result.message = "Face offset successfully";
    result.modified_feature_id = request.targetFeature + "_offset";
    
    double offset_dist = request.offset_params.offset_distance;
    if (request.offset_params.offset_inward) {
        offset_dist = -offset_dist;
    }
    
    result.modified_faces = request.selected_faces;
    double total_area = 0.0;
    
    for (auto& face : result.modified_faces) {
        total_area += face.area;
        
        if (face.normal.size() >= 3) {
            double normal_x = face.normal[0];
            double normal_y = face.normal[1];
            double normal_z = face.normal[2];
            
            double normal_len = std::sqrt(normal_x*normal_x + normal_y*normal_y + normal_z*normal_z);
            if (normal_len > 0.001) {
                normal_x /= normal_len;
                normal_y /= normal_len;
                normal_z /= normal_len;
            }
            
            face.normal[0] = normal_x;
            face.normal[1] = normal_y;
            face.normal[2] = normal_z;
            
            face.area += offset_dist * 0.1;
            if (face.area < 0.0) {
                face.area = 0.0;
            }
        }
    }
    
    result.volume_change = total_area * offset_dist * 0.01;
    result.surface_area_change = total_area * std::abs(offset_dist) * 0.02;
    
    const_cast<DirectEditService*>(this)->updateFeatureHistory(request.targetFeature, result);
    
    return result;
}

DirectEditResult DirectEditService::deleteFace(const DirectEditRequest& request) const {
    DirectEditResult result;
    result.success = true;
    result.message = "Face deleted successfully";
    result.modified_feature_id = request.targetFeature + "_deleted";
    
    double total_area = 0.0;
    double estimated_volume = 0.0;
    
    for (const auto& face : request.selected_faces) {
        total_area += face.area;
        
        if (face.normal.size() >= 3) {
            double normal_len = std::sqrt(
                face.normal[0]*face.normal[0] +
                face.normal[1]*face.normal[1] +
                face.normal[2]*face.normal[2]
            );
            if (normal_len > 0.001) {
                double depth_estimate = std::sqrt(face.area) * 0.5;
                estimated_volume += face.area * depth_estimate;
            }
        }
    }
    
    result.volume_change = -estimated_volume;
    result.surface_area_change = -total_area;
    result.modified_faces.clear();
    
    const_cast<DirectEditService*>(this)->updateFeatureHistory(request.targetFeature, result);
    
    return result;
}

DirectEditResult DirectEditService::freeformEdit(const DirectEditRequest& request) const {
    DirectEditResult result;
    result.success = true;
    result.message = "Freeform edit applied successfully";
    result.modified_feature_id = request.targetFeature + "_freeform";
    
    result.modified_faces = request.selected_faces;
    
    if (request.freeform_params.control_points.empty()) {
        result.volume_change = 0.0;
        result.surface_area_change = 0.0;
        return result;
    }
    
    double tension = request.freeform_params.tension;
    tension = std::max(0.0, std::min(1.0, tension));
    bool smooth = request.freeform_params.smooth;
    
    double total_deformation = 0.0;
    double total_area = 0.0;
    
    for (auto& face : result.modified_faces) {
        if (face.normal.size() < 3) {
            face.normal.resize(3, 0.0);
            face.normal[2] = 1.0;
        }
        
        total_area += face.area;
        
        double base_normal_x = face.normal[0];
        double base_normal_y = face.normal[1];
        double base_normal_z = face.normal[2];
        
        double total_weight = 0.0;
        double new_normal_x = base_normal_x;
        double new_normal_y = base_normal_y;
        double new_normal_z = base_normal_z;
        
        for (const auto& cp : request.freeform_params.control_points) {
            double cp_x = cp.first;
            double cp_y = cp.second;
            
            double dist = std::sqrt(cp_x*cp_x + cp_y*cp_y);
            double weight = 1.0 / (1.0 + dist * dist * (1.0 + tension));
            
            if (smooth) {
                weight = std::exp(-dist * dist / (2.0 * (1.0 - tension + 0.1)));
            }
            
            double influence = cp_x * 0.1 + cp_y * 0.1;
            new_normal_x += influence * weight * 0.1;
            new_normal_y += influence * weight * 0.1;
            new_normal_z += influence * weight * 0.1;
            
            total_weight += weight;
            total_deformation += std::abs(influence) * weight;
        }
        
        if (total_weight > 0.001) {
            new_normal_x /= (1.0 + total_weight * 0.1);
            new_normal_y /= (1.0 + total_weight * 0.1);
            new_normal_z /= (1.0 + total_weight * 0.1);
        }
        
        double normal_len = std::sqrt(new_normal_x*new_normal_x + new_normal_y*new_normal_y + new_normal_z*new_normal_z);
        if (normal_len > 0.001) {
            face.normal[0] = new_normal_x / normal_len;
            face.normal[1] = new_normal_y / normal_len;
            face.normal[2] = new_normal_z / normal_len;
        }
        
        double area_change_factor = 1.0 + total_deformation * 0.05;
        face.area *= area_change_factor;
    }
    
    result.volume_change = total_area * total_deformation * 0.01;
    result.surface_area_change = total_area * total_deformation * 0.02;
    
    const_cast<DirectEditService*>(this)->updateFeatureHistory(request.targetFeature, result);
    
    return result;
            double u = cp.first;
            double v = cp.second;
            
            double dist = std::sqrt(u*u + v*v);
            if (dist < 0.001) {
                continue;
            }
            
            double weight = 1.0 / (dist * dist + 1.0);
            double displacement = request.freeform_params.tension * weight;
            
            new_normal_x += u * displacement * 0.1;
            new_normal_y += v * displacement * 0.1;
            new_normal_z += (u + v) * displacement * 0.05;
            total_weight += weight;
        }
        
        if (total_weight > 0.001) {
            face.normal[0] = new_normal_x / total_weight;
            face.normal[1] = new_normal_y / total_weight;
            face.normal[2] = new_normal_z / total_weight;
            
            double len = std::sqrt(face.normal[0]*face.normal[0] + 
                                  face.normal[1]*face.normal[1] + 
                                  face.normal[2]*face.normal[2]);
            if (len > 0.001) {
                face.normal[0] /= len;
                face.normal[1] /= len;
                face.normal[2] /= len;
            }
        }
    }
    
    const_cast<DirectEditService*>(this)->updateFeatureHistory(request.targetFeature, result);
    
    return result;
}

DirectEditResult DirectEditService::previewEdit(const DirectEditRequest& request) const {
    DirectEditResult result;
    result.success = true;
    result.message = "Edit preview generated";
    
    // Calculate preview changes
    result.volume_change = calculateVolumeChange(request);
    result.surface_area_change = calculateSurfaceAreaChange(request);
    result.modified_faces = request.selected_faces;
    
    return result;
}

void DirectEditService::undoLastEdit() {
    if (canUndo() && history_index_ >= 0) {
        history_index_--;
    }
}

void DirectEditService::redoLastEdit() {
    if (canRedo() && history_index_ < static_cast<int>(edit_history_.size()) - 1) {
        history_index_++;
    }
}

bool DirectEditService::canUndo() const {
    return history_index_ >= 0;
}

bool DirectEditService::canRedo() const {
    return history_index_ < static_cast<int>(edit_history_.size()) - 1;
}

double DirectEditService::calculateVolumeChange(const DirectEditRequest& request) const {
    double volume_change = 0.0;
    
    for (const auto& face : request.selected_faces) {
        double face_area = face.area;
        
        switch (request.operation) {
            case DirectEditOperation::MoveFace: {
                double distance = request.move_params.distance;
                volume_change += face_area * distance;
                break;
            }
            case DirectEditOperation::OffsetFace: {
                double offset = request.offset_params.offset_distance;
                volume_change += face_area * offset;
                break;
            }
            case DirectEditOperation::DeleteFace: {
                // Volume decreases when face is deleted
                volume_change -= face_area * 0.1;  // Simplified
                break;
            }
            case DirectEditOperation::Freeform: {
                // Freeform changes are complex
                volume_change += face_area * 0.05;  // Simplified
                break;
            }
        }
    }
    
    return volume_change;
}

double DirectEditService::calculateSurfaceAreaChange(const DirectEditRequest& request) const {
    double area_change = 0.0;
    
    for (const auto& face : request.selected_faces) {
        switch (request.operation) {
            case DirectEditOperation::MoveFace:
                // Moving face may change adjacent face areas
                area_change += face.area * 0.1;  // Simplified
                break;
            case DirectEditOperation::OffsetFace:
                // Offsetting face changes its area
                area_change += face.area * (request.offset_params.offset_distance / 10.0);
                break;
            case DirectEditOperation::DeleteFace:
                // Deleting face removes its area
                area_change -= face.area;
                break;
            case DirectEditOperation::Freeform:
                // Freeform may increase or decrease area
                area_change += face.area * 0.2;  // Simplified
                break;
        }
    }
    
    return area_change;
}

bool DirectEditService::validateEdit(const DirectEditRequest& request) const {
    if (request.selected_faces.empty()) {
        return false;
    }
    
    switch (request.operation) {
        case DirectEditOperation::MoveFace:
            return request.move_params.distance != 0.0;
        case DirectEditOperation::OffsetFace:
            return request.offset_params.offset_distance != 0.0;
        case DirectEditOperation::DeleteFace:
            return true;
        case DirectEditOperation::Freeform:
            return !request.freeform_params.control_points.empty();
        default:
            return false;
    }
}

void DirectEditService::updateFeatureHistory(const std::string& feature_id, const DirectEditResult& result) {
    std::string history_entry = "Edit: " + result.modified_feature_id + " (" + result.message + ")";
    feature_history_[feature_id].push_back(history_entry);
    
    if (feature_history_[feature_id].size() > 100) {
        feature_history_[feature_id].erase(feature_history_[feature_id].begin());
    }
}

std::vector<std::string> DirectEditService::getFeatureHistory(const std::string& feature_id) const {
    auto it = feature_history_.find(feature_id);
    if (it != feature_history_.end()) {
        return it->second;
    }
    return {};
}

}  // namespace modules
}  // namespace cad
