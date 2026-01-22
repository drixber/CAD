#include <gtest/gtest.h>
#include "modules/patterns/PatternService.h"

using namespace cad::modules;

TEST(PatternServiceTest, CreateRectangularPattern) {
    PatternService service;
    
    PatternRequest request;
    request.targetFeature = "feature_1";
    request.type = PatternType::Rectangular;
    request.rectangular_params.count_x = 3;
    request.rectangular_params.count_y = 2;
    request.rectangular_params.spacing_x = 10.0;
    request.rectangular_params.spacing_y = 10.0;
    
    PatternResult result = service.createPattern(request);
    
    ASSERT_TRUE(result.success);
    EXPECT_EQ(result.pattern_type, PatternType::Rectangular);
    EXPECT_EQ(result.total_instances, 6);
    EXPECT_FALSE(result.instances.empty());
}

TEST(PatternServiceTest, CreateCircularPattern) {
    PatternService service;
    
    PatternRequest request;
    request.targetFeature = "feature_1";
    request.type = PatternType::Circular;
    request.circular_params.count = 8;
    request.circular_params.radius = 20.0;
    request.circular_params.angle = 360.0;
    request.circular_params.center_x = 0.0;
    request.circular_params.center_y = 0.0;
    request.circular_params.center_z = 0.0;
    
    PatternResult result = service.createPattern(request);
    
    ASSERT_TRUE(result.success);
    EXPECT_EQ(result.pattern_type, PatternType::Circular);
    EXPECT_EQ(result.total_instances, 8);
}

TEST(PatternServiceTest, SuppressInstance) {
    PatternService service;
    
    PatternRequest request;
    request.targetFeature = "feature_1";
    request.type = PatternType::Rectangular;
    request.rectangular_params.count_x = 2;
    request.rectangular_params.count_y = 2;
    
    PatternResult create_result = service.createPattern(request);
    ASSERT_TRUE(create_result.success);
    
    std::string pattern_id = create_result.pattern_id;
    std::string instance_id = create_result.instances[0].instance_id;
    
    PatternResult suppress_result = service.suppressInstance(pattern_id, instance_id);
    ASSERT_TRUE(suppress_result.success);
    
    std::vector<PatternInstance> instances = service.getPatternInstances(pattern_id);
    bool found_suppressed = false;
    for (const auto& inst : instances) {
        if (inst.instance_id == instance_id && inst.suppressed) {
            found_suppressed = true;
            break;
        }
    }
    EXPECT_TRUE(found_suppressed);
}

TEST(PatternServiceTest, DeletePattern) {
    PatternService service;
    
    PatternRequest request;
    request.targetFeature = "feature_1";
    request.type = PatternType::Rectangular;
    request.rectangular_params.count_x = 2;
    request.rectangular_params.count_y = 2;
    
    PatternResult create_result = service.createPattern(request);
    ASSERT_TRUE(create_result.success);
    
    std::string pattern_id = create_result.pattern_id;
    PatternResult delete_result = service.deletePattern(pattern_id);
    ASSERT_TRUE(delete_result.success);
    
    std::vector<PatternInstance> instances = service.getPatternInstances(pattern_id);
    EXPECT_TRUE(instances.empty());
}

