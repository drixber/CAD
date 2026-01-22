#pragma once

#include <string>
#include <vector>

#include "PmiData.h"

namespace cad {
namespace modules {

struct MbdRequest {
    std::string targetPart;
    std::string note;
};

struct MbdResult {
    bool success{false};
    std::string message;
};

struct MbdRenderRequest {
    std::string part_id;
    bool show_annotations{true};
    bool show_datums{true};
    bool show_tolerances{true};
    double view_scale{1.0};
};

struct MbdRenderResult {
    bool success{false};
    std::vector<cad::mbd::PmiAnnotation> visible_annotations;
    std::vector<cad::mbd::PmiDatum> visible_datums;
    std::vector<cad::mbd::PmiTolerance> visible_tolerances;
};

class MbdService {
public:
    MbdResult applyMbd(const MbdRequest& request) const;
    cad::mbd::PmiDataSet buildDefaultPmi(const std::string& part_id) const;
    MbdRenderResult prepareForRendering(const MbdRenderRequest& request) const;
    std::vector<cad::mbd::PmiAnnotation> getVisibleAnnotations(const cad::mbd::PmiDataSet& pmi_data, bool show_annotations) const;
    std::vector<cad::mbd::PmiDatum> getVisibleDatums(const cad::mbd::PmiDataSet& pmi_data, bool show_datums) const;
    std::vector<cad::mbd::PmiTolerance> getVisibleTolerances(const cad::mbd::PmiDataSet& pmi_data, bool show_tolerances) const;
    
    // Viewport integration
    void renderMbdInViewport(const MbdRenderResult& render_data, void* viewport_handle) const;
    void updateMbdVisibility(const std::string& part_id, bool show_annotations, bool show_datums, bool show_tolerances) const;
    std::vector<cad::mbd::PmiAnnotation> getAnnotationsForViewport(const std::string& part_id, double view_scale) const;
    
private:
    struct PmiColor {
        double r{0.0};
        double g{0.0};
        double b{0.0};
        double a{1.0};
    };
    
    struct PmiAnnotationRenderData {
        std::string text;
        double x{0.0};
        double y{0.0};
        double z{0.0};
        double font_size{2.5};
        PmiColor color;
        bool has_leader{false};
        double leader_start_x{0.0};
        double leader_start_y{0.0};
        double leader_start_z{0.0};
        double leader_end_x{0.0};
        double leader_end_y{0.0};
        double leader_end_z{0.0};
    };
    
    struct PmiDatumRenderData {
        std::string id;
        std::string type;
        std::string description;
    };
    
    struct PmiToleranceRenderData {
        std::string label;
        double upper{0.0};
        double lower{0.0};
        std::string units;
    };
    
    struct PmiRenderData {
        std::vector<PmiAnnotationRenderData> annotations;
        std::vector<PmiDatumRenderData> datums;
        std::vector<PmiToleranceRenderData> tolerances;
        int annotation_count{0};
        int datum_count{0};
        int tolerance_count{0};
    };
    
    struct VisibilityState {
        bool show_annotations{true};
        bool show_datums{true};
        bool show_tolerances{true};
        int annotation_count{0};
        int datum_count{0};
        int tolerance_count{0};
    };
    
    mutable std::map<void*, PmiRenderData> rendered_pmi_data_;
    mutable std::map<std::string, VisibilityState> visibility_state_;
    mutable std::map<std::string, cad::mbd::PmiDataSet> pmi_data_cache_;
    
    PmiColor parseColor(const std::string& color_str) const;
};

}  // namespace modules
}  // namespace cad
