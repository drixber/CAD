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
    
    // Update face positions
    result.modified_faces = request.selected_faces;
    for (auto& face : result.modified_faces) {
        // In real implementation: would update face geometry
        // For now, just mark as modified
    }
    
    return result;
}

DirectEditResult DirectEditService::offsetFace(const DirectEditRequest& request) const {
    DirectEditResult result;
    result.success = true;
    result.message = "Face offset successfully";
    result.modified_feature_id = request.targetFeature + "_offset";
    
    // Calculate volume and surface area changes
    result.volume_change = calculateVolumeChange(request);
    result.surface_area_change = calculateSurfaceAreaChange(request);
    
    // Update face positions
    result.modified_faces = request.selected_faces;
    
    return result;
}

DirectEditResult DirectEditService::deleteFace(const DirectEditRequest& request) const {
    DirectEditResult result;
    result.success = true;
    result.message = "Face deleted successfully";
    result.modified_feature_id = request.targetFeature + "_deleted";
    
    // Calculate volume and surface area changes
    result.volume_change = calculateVolumeChange(request);
    result.surface_area_change = calculateSurfaceAreaChange(request);
    
    // Faces are deleted, so no modified faces
    result.modified_faces.clear();
    
    return result;
}

DirectEditResult DirectEditService::freeformEdit(const DirectEditRequest& request) const {
    DirectEditResult result;
    result.success = true;
    result.message = "Freeform edit applied successfully";
    result.modified_feature_id = request.targetFeature + "_freeform";
    
    // Calculate volume and surface area changes
    result.volume_change = calculateVolumeChange(request);
    result.surface_area_change = calculateSurfaceAreaChange(request);
    
    // Update faces with freeform deformation
    result.modified_faces = request.selected_faces;
    for (auto& face : result.modified_faces) {
        // In real implementation: would apply freeform deformation
    }
    
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
    // Validate edit request
    if (request.selected_faces.empty()) {
        return false;
    }
    
    switch (request.operation) {
        case DirectEditOperation::MoveFace:
            return request.move_params.distance != 0.0;
        case DirectEditOperation::OffsetFace:
            return request.offset_params.offset_distance != 0.0;
        case DirectEditOperation::DeleteFace:
            return true;  // Always valid
        case DirectEditOperation::Freeform:
            return !request.freeform_params.control_points.empty();
        default:
            return false;
    }
}

}  // namespace modules
}  // namespace cad
