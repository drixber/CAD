#include <gtest/gtest.h>
#include "modules/sheetmetal/SheetMetalService.h"

using namespace cad::modules;

TEST(SheetMetalServiceTest, CreateFlange) {
    SheetMetalService service;
    
    SheetMetalRequest request;
    request.targetPart = "test_part";
    request.operation = SheetMetalOperation::Flange;
    request.flange_params.height = 20.0;
    request.flange_params.angle = 90.0;
    
    service.setMaterialThickness("test_part", 2.0);
    
    SheetMetalResult result = service.createFlange(request);
    
    ASSERT_TRUE(result.success);
    ASSERT_FALSE(result.modified_part_id.empty());
    ASSERT_GT(result.flat_pattern_length, 0.0);
}

TEST(SheetMetalServiceTest, CreateBend) {
    SheetMetalService service;
    
    SheetMetalRequest request;
    request.targetPart = "test_part";
    request.operation = SheetMetalOperation::Bend;
    request.bend_params.angle = 90.0;
    request.bend_params.radius = 5.0;
    request.bend_params.k_factor = 0.5;
    
    service.setMaterialThickness("test_part", 2.0);
    
    SheetMetalResult result = service.createBend(request);
    
    ASSERT_TRUE(result.success);
    ASSERT_FALSE(result.resulting_bend_lines.empty());
}

TEST(SheetMetalServiceTest, UnfoldRefold) {
    SheetMetalService service;
    
    SheetMetalRequest unfold_request;
    unfold_request.targetPart = "test_part";
    unfold_request.operation = SheetMetalOperation::Unfold;
    
    BendLine bend_line;
    bend_line.angle = 90.0;
    bend_line.radius = 5.0;
    unfold_request.bend_lines.push_back(bend_line);
    
    service.setMaterialThickness("test_part", 2.0);
    
    SheetMetalResult unfold_result = service.unfoldSheet(unfold_request);
    ASSERT_TRUE(unfold_result.success);
    
    SheetMetalRequest refold_request;
    refold_request.targetPart = "test_part";
    refold_request.operation = SheetMetalOperation::Refold;
    refold_request.bend_lines = unfold_result.resulting_bend_lines;
    
    SheetMetalResult refold_result = service.refoldSheet(refold_request);
    ASSERT_TRUE(refold_result.success);
}

TEST(SheetMetalServiceTest, BendAllowanceCalculation) {
    SheetMetalService service;
    
    double allowance = service.calculateBendAllowance(90.0, 5.0, 2.0, 0.5);
    ASSERT_GT(allowance, 0.0);
    
    double deduction = service.calculateBendDeduction(90.0, 5.0, 2.0, 0.5);
    ASSERT_GT(deduction, 0.0);
}

TEST(SheetMetalServiceTest, FlatPatternGeneration) {
    SheetMetalService service;
    
    service.setMaterialThickness("test_part", 2.0);
    service.setKFactor("test_part", 0.5);
    
    SheetMetalResult result = service.generateFlatPattern("test_part");
    
    ASSERT_TRUE(result.success);
    ASSERT_GT(result.flat_pattern_length, 0.0);
    ASSERT_GT(result.flat_pattern_width, 0.0);
}
