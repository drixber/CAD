#include "MbdService.h"

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
    
    // In a real implementation, this would load PMI data from the part
    // For now, we use default PMI data
    cad::mbd::PmiDataSet pmi_data = buildDefaultPmi(request.part_id);
    
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

}  // namespace modules
}  // namespace cad
