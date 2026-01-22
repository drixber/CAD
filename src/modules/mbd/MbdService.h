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
};

}  // namespace modules
}  // namespace cad
