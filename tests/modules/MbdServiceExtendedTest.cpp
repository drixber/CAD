#include <gtest/gtest.h>
#include "modules/mbd/MbdService.h"

using namespace cad::modules;

TEST(MbdServiceExtendedTest, PMIRendering) {
    MbdService service;
    
    MbdRenderRequest request;
    request.part_id = "test_part";
    request.show_annotations = true;
    request.show_datums = true;
    request.show_tolerances = true;
    
    MbdRenderResult result = service.prepareForRendering(request);
    
    ASSERT_TRUE(result.success);
}

TEST(MbdServiceExtendedTest, DatumSymbols) {
    MbdService service;
    
    MbdRenderRequest request;
    request.part_id = "test_part";
    request.show_datums = true;
    
    MbdRenderResult result = service.prepareForRendering(request);
    
    ASSERT_FALSE(result.visible_datums.empty());
}

TEST(MbdServiceExtendedTest, ToleranceCallouts) {
    MbdService service;
    
    MbdRenderRequest request;
    request.part_id = "test_part";
    request.show_tolerances = true;
    
    MbdRenderResult result = service.prepareForRendering(request);
    
    ASSERT_FALSE(result.visible_tolerances.empty());
}

TEST(MbdServiceExtendedTest, LeaderLines) {
    MbdService service;
    
    cad::mbd::PmiDataSet pmi_data = service.buildDefaultPmi("test_part");
    
    std::vector<cad::mbd::PmiAnnotation> annotations = service.getVisibleAnnotations(pmi_data, true);
    
    ASSERT_FALSE(annotations.empty());
    for (const auto& ann : annotations) {
        if (ann.show_leader) {
            ASSERT_NE(ann.leader_x, 0.0);
            ASSERT_NE(ann.leader_y, 0.0);
        }
    }
}
