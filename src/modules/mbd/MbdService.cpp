#include "MbdService.h"

#include <map>
#include <string>
#include <sstream>
#include <iomanip>

namespace cad {
namespace modules {

MbdResult MbdService::applyMbd(const MbdRequest& request) const {
    MbdResult result;
    if (!request.targetPart.empty()) {
        result.success = true;
        result.message = "MBD applied";
    }
    return result;
}

cad::mbd::PmiDataSet MbdService::buildDefaultPmi(const std::string& part_id) const {
    cad::mbd::PmiDataSet data;
    data.part_id = part_id;
    data.datums.push_back({"A", "Plane", "Primary datum"});
    data.tolerances.push_back({"Profile", 0.1, -0.1, "mm"});
    
    cad::mbd::PmiAnnotation annotation;
    annotation.text = "PMI note";
    annotation.x = 0.0;
    annotation.y = 0.0;
    annotation.z = 0.0;
    annotation.visibility = cad::mbd::PmiAnnotationVisibility::Always;
    annotation.font_size = 2.5;
    annotation.color = "black";
    annotation.show_leader = true;
    data.annotations.push_back(annotation);
    
    return data;
}

MbdRenderResult MbdService::prepareForRendering(const MbdRenderRequest& request) const {
    MbdRenderResult result;
    
    auto it = pmi_data_cache_.find(request.part_id);
    cad::mbd::PmiDataSet pmi_data;
    
    if (it != pmi_data_cache_.end()) {
        pmi_data = it->second;
    } else {
        pmi_data = buildDefaultPmi(request.part_id);
        pmi_data_cache_[request.part_id] = pmi_data;
    }
    
    result.visible_annotations = getVisibleAnnotations(pmi_data, request.show_annotations);
    result.visible_datums = getVisibleDatums(pmi_data, request.show_datums);
    result.visible_tolerances = getVisibleTolerances(pmi_data, request.show_tolerances);
    
    result.success = !result.visible_annotations.empty() || 
                     !result.visible_datums.empty() || 
                     !result.visible_tolerances.empty();
    
    return result;
}

std::vector<cad::mbd::PmiAnnotation> MbdService::getVisibleAnnotations(const cad::mbd::PmiDataSet& pmi_data, bool show_annotations) const {
    std::vector<cad::mbd::PmiAnnotation> visible;
    if (!show_annotations) {
        return visible;
    }
    
    for (const auto& annotation : pmi_data.annotations) {
        if (annotation.visibility == cad::mbd::PmiAnnotationVisibility::Always ||
            annotation.visibility == cad::mbd::PmiAnnotationVisibility::OnSelection ||
            annotation.visibility == cad::mbd::PmiAnnotationVisibility::OnHover) {
            visible.push_back(annotation);
        }
    }
    
    return visible;
}

std::vector<cad::mbd::PmiDatum> MbdService::getVisibleDatums(const cad::mbd::PmiDataSet& pmi_data, bool show_datums) const {
    std::vector<cad::mbd::PmiDatum> visible;
    if (!show_datums) {
        return visible;
    }
    
    // All datums are visible by default in MBD
    return pmi_data.datums;
}

std::vector<cad::mbd::PmiTolerance> MbdService::getVisibleTolerances(const cad::mbd::PmiDataSet& pmi_data, bool show_tolerances) const {
    std::vector<cad::mbd::PmiTolerance> visible;
    if (!show_tolerances) {
        return visible;
    }
    
    // All tolerances are visible by default in MBD
    return pmi_data.tolerances;
}

void MbdService::renderMbdInViewport(const MbdRenderResult& render_data, void* viewport_handle) const {
    if (!viewport_handle) {
        return;
    }
    
    PmiRenderData render_info;
    render_info.annotation_count = static_cast<int>(render_data.visible_annotations.size());
    render_info.datum_count = static_cast<int>(render_data.visible_datums.size());
    render_info.tolerance_count = static_cast<int>(render_data.visible_tolerances.size());
    
    for (const auto& annotation : render_data.visible_annotations) {
        PmiAnnotationRenderData ann_data;
        ann_data.text = annotation.text;
        ann_data.x = annotation.x;
        ann_data.y = annotation.y;
        ann_data.z = annotation.z;
        ann_data.font_size = annotation.font_size;
        ann_data.color = parseColor(annotation.color);
        ann_data.has_leader = annotation.show_leader;
        
        if (annotation.show_leader) {
            if (annotation.leader_x != 0.0 || annotation.leader_y != 0.0 || annotation.leader_z != 0.0) {
                ann_data.leader_start_x = annotation.leader_x;
                ann_data.leader_start_y = annotation.leader_y;
                ann_data.leader_start_z = annotation.leader_z;
            } else {
                ann_data.leader_start_x = annotation.x - 10.0;
                ann_data.leader_start_y = annotation.y - 10.0;
                ann_data.leader_start_z = annotation.z;
            }
            ann_data.leader_end_x = annotation.x;
            ann_data.leader_end_y = annotation.y;
            ann_data.leader_end_z = annotation.z;
        }
        
        render_info.annotations.push_back(ann_data);
    }
    
    for (const auto& datum : render_data.visible_datums) {
        PmiDatumRenderData datum_data;
        datum_data.id = datum.id;
        datum_data.type = datum.type;
        datum_data.description = datum.description;
        
        // Datum-Symbol-Rendering: Create geometry for datum frame
        // In real implementation, this would create Coin3D geometry nodes
        // for datum frames (rectangular frames with datum letter)
        
        render_info.datums.push_back(datum_data);
    }
    
    for (const auto& tolerance : render_data.visible_tolerances) {
        PmiToleranceRenderData tol_data;
        tol_data.label = tolerance.label;
        tol_data.upper = tolerance.upper;
        tol_data.lower = tolerance.lower;
        tol_data.units = tolerance.units;
        
        // Tolerance-Callout-Rendering: Create geometry for tolerance frame
        // In real implementation, this would create Coin3D geometry nodes
        // for tolerance frames (feature control frames)
        
        render_info.tolerances.push_back(tol_data);
    }
    
    rendered_pmi_data_[viewport_handle] = render_info;
}

MbdService::PmiColor MbdService::parseColor(const std::string& color_str) const {
    PmiColor color;
    color.r = 0.0;
    color.g = 0.0;
    color.b = 0.0;
    color.a = 1.0;
    
    if (color_str.empty()) {
        return color;
    }
    
    if (color_str[0] == '#') {
        if (color_str.length() >= 7) {
            int r = std::stoi(color_str.substr(1, 2), nullptr, 16);
            int g = std::stoi(color_str.substr(3, 2), nullptr, 16);
            int b = std::stoi(color_str.substr(5, 2), nullptr, 16);
            color.r = r / 255.0;
            color.g = g / 255.0;
            color.b = b / 255.0;
        }
    } else if (color_str == "black") {
        color.r = 0.0; color.g = 0.0; color.b = 0.0;
    } else if (color_str == "white") {
        color.r = 1.0; color.g = 1.0; color.b = 1.0;
    } else if (color_str == "red") {
        color.r = 1.0; color.g = 0.0; color.b = 0.0;
    } else if (color_str == "green") {
        color.r = 0.0; color.g = 1.0; color.b = 0.0;
    } else if (color_str == "blue") {
        color.r = 0.0; color.g = 0.0; color.b = 1.0;
    }
    
    return color;
}

void MbdService::updateMbdVisibility(const std::string& part_id, bool show_annotations, bool show_datums, bool show_tolerances) const {
    MbdRenderRequest request;
    request.part_id = part_id;
    request.show_annotations = show_annotations;
    request.show_datums = show_datums;
    request.show_tolerances = show_tolerances;
    
    MbdRenderResult result = prepareForRendering(request);
    
    visibility_state_[part_id] = {
        show_annotations,
        show_datums,
        show_tolerances,
        static_cast<int>(result.visible_annotations.size()),
        static_cast<int>(result.visible_datums.size()),
        static_cast<int>(result.visible_tolerances.size())
    };
}

std::vector<cad::mbd::PmiAnnotation> MbdService::getAnnotationsForViewport(const std::string& part_id, double view_scale) const {
    // Get annotations scaled for viewport display
    MbdRenderRequest request;
    request.part_id = part_id;
    request.view_scale = view_scale;
    request.show_annotations = true;
    
    MbdRenderResult result = prepareForRendering(request);
    
    // Scale annotations based on view scale
    for (auto& annotation : result.visible_annotations) {
        annotation.font_size *= view_scale;
        // Scale leader line positions if needed
    }
    
    return result.visible_annotations;
}

}  // namespace modules
}  // namespace cad
