#include <gtest/gtest.h>
#include "modules/visualization/VisualizationService.h"

using namespace cad::modules;

TEST(VisualizationServiceExtendedTest, IllustrationMode) {
    VisualizationService service;
    
    VisualizationRequest request;
    request.targetPart = "test_part";
    request.mode = VisualizationMode::Illustration;
    request.render_settings.quality = RenderQuality::High;
    request.render_settings.background_color = "#ffffff";
    
    VisualizationResult result = service.createIllustration(request);
    
    ASSERT_TRUE(result.success);
    ASSERT_EQ(result.frame_count, 1);
    ASSERT_LT(result.render_time, 1.0);
}

TEST(VisualizationServiceExtendedTest, PhotorealisticRendering) {
    VisualizationService service;
    
    VisualizationRequest request;
    request.targetPart = "test_part";
    request.mode = VisualizationMode::Rendering;
    request.render_settings.quality = RenderQuality::Ultra;
    request.render_settings.enable_shadows = true;
    request.render_settings.enable_reflections = true;
    
    VisualizationResult result = service.createRendering(request);
    
    ASSERT_TRUE(result.success);
    ASSERT_GT(result.render_time, 1.0);
}

TEST(VisualizationServiceExtendedTest, KeyframeAnimation) {
    VisualizationService service;
    
    VisualizationRequest request;
    request.targetPart = "test_part";
    request.mode = VisualizationMode::Animation;
    request.animation_settings.duration = 5.0;
    request.animation_settings.frame_rate = 30;
    request.animation_settings.keyframes = {"keyframe1", "keyframe2", "keyframe3"};
    
    VisualizationResult result = service.createAnimation(request);
    
    ASSERT_TRUE(result.success);
    ASSERT_EQ(result.frame_count, 150);
    ASSERT_GT(result.render_time, 0.0);
}
