#include <gtest/gtest.h>
#include "modules/mbd/MbdService.h"

using namespace cad::modules;

TEST(MbdServiceTest, BuildDefaultPmi) {
    MbdService service;
    
    cad::mbd::PmiDataSet pmi_data = service.buildDefaultPmi("part_1");
    
    EXPECT_EQ(pmi_data.part_id, "part_1");
    EXPECT_FALSE(pmi_data.datums.empty());
    EXPECT_FALSE(pmi_data.tolerances.empty());
    EXPECT_FALSE(pmi_data.annotations.empty());
}

TEST(MbdServiceTest, PrepareForRendering) {
    MbdService service;
    
    MbdRenderRequest request;
    request.part_id = "part_1";
    request.show_annotations = true;
    request.show_datums = true;
    request.show_tolerances = true;
    
    MbdRenderResult result = service.prepareForRendering(request);
    
    ASSERT_TRUE(result.success);
    EXPECT_FALSE(result.visible_annotations.empty());
    EXPECT_FALSE(result.visible_datums.empty());
    EXPECT_FALSE(result.visible_tolerances.empty());
}

TEST(MbdServiceTest, GetVisibleAnnotations) {
    MbdService service;
    
    cad::mbd::PmiDataSet pmi_data = service.buildDefaultPmi("part_1");
    std::vector<cad::mbd::PmiAnnotation> visible = service.getVisibleAnnotations(pmi_data, true);
    
    EXPECT_FALSE(visible.empty());
    
    std::vector<cad::mbd::PmiAnnotation> hidden = service.getVisibleAnnotations(pmi_data, false);
    EXPECT_TRUE(hidden.empty());
}

TEST(MbdServiceTest, GetAnnotationsForViewport) {
    MbdService service;
    
    std::vector<cad::mbd::PmiAnnotation> annotations = service.getAnnotationsForViewport("part_1", 2.0);
    
    EXPECT_FALSE(annotations.empty());
    for (const auto& ann : annotations) {
        EXPECT_GT(ann.font_size, 0.0);
    }
}

TEST(MbdServiceTest, UpdateMbdVisibility) {
    MbdService service;
    
    service.updateMbdVisibility("part_1", true, false, true);
    
    MbdRenderRequest request;
    request.part_id = "part_1";
    request.show_annotations = true;
    request.show_datums = false;
    request.show_tolerances = true;
    
    MbdRenderResult result = service.prepareForRendering(request);
    ASSERT_TRUE(result.success);
    EXPECT_FALSE(result.visible_annotations.empty());
    EXPECT_TRUE(result.visible_datums.empty());
    EXPECT_FALSE(result.visible_tolerances.empty());
}

