#pragma once

#include <string>
#include <vector>
#include <map>

namespace cad {
namespace modules {

enum class DirectEditOperation {
    MoveFace,
    OffsetFace,
    DeleteFace,
    Freeform
};

struct FaceSelection {
    std::string face_id;
    std::string part_id;
    std::vector<double> normal;  // Face normal vector
    double area{0.0};
};

struct MoveFaceParams {
    double distance{0.0};
    std::vector<double> direction;  // Direction vector
    bool keep_connected{true};
};

struct OffsetFaceParams {
    double offset_distance{0.0};
    bool offset_inward{false};
};

struct FreeformParams {
    std::vector<std::pair<double, double>> control_points;
    double tension{0.5};
    bool smooth{true};
};

struct DirectEditRequest {
    std::string targetFeature;
    DirectEditOperation operation;
    std::vector<FaceSelection> selected_faces;
    MoveFaceParams move_params;
    OffsetFaceParams offset_params;
    FreeformParams freeform_params;
    bool preview_mode{false};
};

struct DirectEditResult {
    bool success{false};
    std::string message;
    std::string modified_feature_id;
    std::vector<FaceSelection> modified_faces;
    double volume_change{0.0};
    double surface_area_change{0.0};
};

class DirectEditService {
public:
    DirectEditResult applyEdit(const DirectEditRequest& request) const;
    DirectEditResult moveFace(const DirectEditRequest& request) const;
    DirectEditResult offsetFace(const DirectEditRequest& request) const;
    DirectEditResult deleteFace(const DirectEditRequest& request) const;
    DirectEditResult freeformEdit(const DirectEditRequest& request) const;
    
    // Preview
    DirectEditResult previewEdit(const DirectEditRequest& request) const;
    
    // History
    void undoLastEdit();
    void redoLastEdit();
    bool canUndo() const;
    bool canRedo() const;
    void updateFeatureHistory(const std::string& feature_id, const DirectEditResult& result);
    std::vector<std::string> getFeatureHistory(const std::string& feature_id) const;
    
private:
    std::vector<DirectEditResult> edit_history_;
    int history_index_{-1};
    std::map<std::string, std::vector<std::string>> feature_history_;
    
    double calculateVolumeChange(const DirectEditRequest& request) const;
    double calculateSurfaceAreaChange(const DirectEditRequest& request) const;
    bool validateEdit(const DirectEditRequest& request) const;
};

}  // namespace modules
}  // namespace cad
