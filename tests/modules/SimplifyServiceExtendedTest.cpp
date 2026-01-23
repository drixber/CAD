#include <gtest/gtest.h>
#include "modules/simplify/SimplifyService.h"

using namespace cad::modules;

TEST(SimplifyServiceExtendedTest, FeatureRemoval) {
    SimplifyService service;
    
    SimplifyRequest request;
    request.targetAssembly = "test_assembly";
    request.mode = SimplifyMode::RemoveInternalFeatures;
    request.detail_threshold = 2.0;
    
    SimplifyResult result = service.removeInternalFeatures(request.targetAssembly, request.detail_threshold);
    
    ASSERT_TRUE(result.success);
    ASSERT_GT(result.file_size_reduction, 0.0);
}

TEST(SimplifyServiceExtendedTest, SmallFeatureRemoval) {
    SimplifyService service;
    
    SimplifyRequest request;
    request.targetAssembly = "test_assembly";
    request.mode = SimplifyMode::RemoveSmallFeatures;
    request.detail_threshold = 1.0;
    
    SimplifyResult result = service.removeSmallFeatures(request.targetAssembly, request.detail_threshold);
    
    ASSERT_TRUE(result.success);
    ASSERT_GT(result.performance_improvement, 0.0);
}

TEST(SimplifyServiceExtendedTest, GeometryComplexity) {
    SimplifyService service;
    
    double ratio = service.calculateGeometryComplexityRatio("original_part", "simplified_part");
    
    ASSERT_GE(ratio, 0.0);
    ASSERT_LE(ratio, 1.0);
}
