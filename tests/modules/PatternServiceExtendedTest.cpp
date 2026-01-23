#include <gtest/gtest.h>
#include "modules/patterns/PatternService.h"

using namespace cad::modules;

TEST(PatternServiceExtendedTest, TransformMatrices) {
    PatternService service;
    
    PatternRequest request;
    request.targetFeature = "test_feature";
    request.type = PatternType::Rectangular;
    request.rectangular_params.count_x = 3;
    request.rectangular_params.count_y = 3;
    request.rectangular_params.spacing_x = 10.0;
    request.rectangular_params.spacing_y = 10.0;
    
    PatternResult result = service.createRectangularPattern(request);
    
    ASSERT_TRUE(result.success);
    ASSERT_EQ(result.instances.size(), 9);
    
    for (const auto& instance : result.instances) {
        ASSERT_TRUE(instance.has_transform_matrix);
        ASSERT_NE(instance.transform_matrix[0], 0.0);
    }
}

TEST(PatternServiceExtendedTest, CircularPatternTransforms) {
    PatternService service;
    
    PatternRequest request;
    request.targetFeature = "test_feature";
    request.type = PatternType::Circular;
    request.circular_params.count = 8;
    request.circular_params.radius = 20.0;
    request.circular_params.angle = 360.0;
    
    PatternResult result = service.createCircularPattern(request);
    
    ASSERT_TRUE(result.success);
    ASSERT_EQ(result.instances.size(), 8);
    
    for (const auto& instance : result.instances) {
        ASSERT_TRUE(instance.has_transform_matrix);
    }
}
