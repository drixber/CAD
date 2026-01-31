#include <gtest/gtest.h>
#include "modules/simplify/SimplifyService.h"

using namespace cad::modules;

TEST(SimplifyServiceTest, ReplaceWithBoundingBox) {
    SimplifyService service;
    
    SimplifyResult result = service.replaceWithBoundingBox("assembly_1");
    
    ASSERT_TRUE(result.success);
    EXPECT_GT(result.simplified_component_count, 0);
    EXPECT_GT(result.file_size_reduction, 0.0);
    EXPECT_GT(result.performance_improvement, 0.0);
}

TEST(SimplifyServiceTest, RemoveSmallFeatures) {
    SimplifyService service;
    
    SimplifyResult result = service.removeSmallFeatures("assembly_1", 1.0);
    
    ASSERT_TRUE(result.success);
    EXPECT_GT(result.simplified_component_count, 0);
    EXPECT_GT(result.file_size_reduction, 0.0);
}

TEST(SimplifyServiceTest, GetSimplificationRatio) {
    SimplifyService service;
    
    SimplifyResult result = service.replaceWithBoundingBox("assembly_1");
    ASSERT_TRUE(result.success);
    
    double ratio = service.getSimplificationRatio("assembly_1");
    EXPECT_GE(ratio, 0.0);
    EXPECT_LE(ratio, 1.0);
}

TEST(SimplifyServiceTest, CreatePresets) {
    SimplifyService service;
    
    SimplifyRequest perf_preset = service.createPerformancePreset();
    EXPECT_EQ(perf_preset.mode, SimplifyMode::ReplaceWithBoundingBox);
    EXPECT_GT(perf_preset.detail_threshold, 0.0);
    
    SimplifyRequest detail_preset = service.createDetailPreset();
    EXPECT_EQ(detail_preset.mode, SimplifyMode::RemoveSmallFeatures);
    EXPECT_LT(detail_preset.detail_threshold, perf_preset.detail_threshold);
    
    SimplifyRequest balanced_preset = service.createBalancedPreset();
    EXPECT_EQ(balanced_preset.mode, SimplifyMode::ReplaceWithSimplifiedGeometry);
}

