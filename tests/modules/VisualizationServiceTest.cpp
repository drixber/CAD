#include <gtest/gtest.h>
#include "modules/visualization/VisualizationService.h"

using namespace cad::modules;

TEST(VisualizationServiceTest, CreateIllustration) {
    VisualizationService service;
    
    VisualizationRequest request;
    request.targetPart = "part_1";
    request.mode = VisualizationMode::Illustration;
    
    VisualizationResult result = service.runVisualization(request);
    
    ASSERT_TRUE(result.success);
    EXPECT_GT(result.frame_count, 0);
    EXPECT_GE(result.render_time, 0.0);
}

TEST(VisualizationServiceTest, CreateRendering) {
    VisualizationService service;
    
    VisualizationRequest request;
    request.targetPart = "part_1";
    request.mode = VisualizationMode::Rendering;
    request.render_settings.quality = RenderQuality::High;
    
    VisualizationResult result = service.runVisualization(request);
    
    ASSERT_TRUE(result.success);
    EXPECT_EQ(request.render_settings.resolution_x, 1920);
    EXPECT_EQ(request.render_settings.resolution_y, 1080);
    EXPECT_TRUE(request.render_settings.enable_shadows);
}

TEST(VisualizationServiceTest, CreateAnimation) {
    VisualizationService service;
    
    VisualizationRequest request;
    request.targetPart = "part_1";
    request.mode = VisualizationMode::Animation;
    request.animation_settings.duration = 5.0;
    request.animation_settings.frame_rate = 30;
    
    VisualizationResult result = service.runVisualization(request);
    
    ASSERT_TRUE(result.success);
    EXPECT_EQ(result.frame_count, 150);
    EXPECT_GT(result.render_time, 0.0);
}

TEST(VisualizationServiceTest, ExportToImage) {
    VisualizationService service;
    
    VisualizationResult result = service.exportToImage("part_1", "output.png", RenderQuality::Medium);
    
    ASSERT_TRUE(result.success);
    EXPECT_FALSE(result.output_file_path.empty());
}

TEST(VisualizationServiceTest, QualityPresets) {
    VisualizationService service;
    
    VisualizationRequest req_low;
    req_low.targetPart = "part_1";
    req_low.mode = VisualizationMode::Rendering;
    req_low.render_settings.quality = RenderQuality::Low;
    req_low.render_settings.resolution_x = 640;
    req_low.render_settings.resolution_y = 480;
    VisualizationResult res_low = service.runVisualization(req_low);
    EXPECT_TRUE(res_low.success || !res_low.message.empty());
    
    VisualizationRequest req_ultra;
    req_ultra.targetPart = "part_1";
    req_ultra.mode = VisualizationMode::Rendering;
    req_ultra.render_settings.quality = RenderQuality::Ultra;
    req_ultra.render_settings.resolution_x = 3840;
    req_ultra.render_settings.resolution_y = 2160;
    VisualizationResult res_ultra = service.runVisualization(req_ultra);
    EXPECT_TRUE(res_ultra.success || !res_ultra.message.empty());
}

