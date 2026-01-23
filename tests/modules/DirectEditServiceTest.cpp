#include <gtest/gtest.h>
#include "modules/direct/DirectEditService.h"

using namespace cad::modules;

TEST(DirectEditServiceTest, MoveFace) {
    DirectEditService service;
    
    DirectEditRequest request;
    request.targetFeature = "test_feature";
    request.operation = DirectEditOperation::MoveFace;
    
    FaceSelection face;
    face.face_id = "face1";
    face.part_id = "part1";
    face.normal = {0.0, 0.0, 1.0};
    face.area = 100.0;
    request.selected_faces.push_back(face);
    
    request.move_params.distance = 10.0;
    request.move_params.direction = {1.0, 0.0, 0.0};
    
    DirectEditResult result = service.moveFace(request);
    
    ASSERT_TRUE(result.success);
    ASSERT_FALSE(result.modified_faces.empty());
}

TEST(DirectEditServiceTest, OffsetFace) {
    DirectEditService service;
    
    DirectEditRequest request;
    request.targetFeature = "test_feature";
    request.operation = DirectEditOperation::OffsetFace;
    
    FaceSelection face;
    face.face_id = "face1";
    face.normal = {0.0, 0.0, 1.0};
    face.area = 100.0;
    request.selected_faces.push_back(face);
    
    request.offset_params.offset_distance = 5.0;
    request.offset_params.offset_inward = false;
    
    DirectEditResult result = service.offsetFace(request);
    
    ASSERT_TRUE(result.success);
}

TEST(DirectEditServiceTest, FreeformEdit) {
    DirectEditService service;
    
    DirectEditRequest request;
    request.targetFeature = "test_feature";
    request.operation = DirectEditOperation::Freeform;
    
    FaceSelection face;
    face.face_id = "face1";
    face.normal = {0.0, 0.0, 1.0};
    face.area = 100.0;
    request.selected_faces.push_back(face);
    
    request.freeform_params.control_points = {{0.0, 0.0}, {1.0, 1.0}, {2.0, 0.0}};
    request.freeform_params.tension = 0.5;
    request.freeform_params.smooth = true;
    
    DirectEditResult result = service.freeformEdit(request);
    
    ASSERT_TRUE(result.success);
}

TEST(DirectEditServiceTest, FeatureHistory) {
    DirectEditService service;
    
    DirectEditRequest request;
    request.targetFeature = "test_feature";
    request.operation = DirectEditOperation::MoveFace;
    
    FaceSelection face;
    face.face_id = "face1";
    request.selected_faces.push_back(face);
    
    DirectEditResult result = service.moveFace(request);
    service.updateFeatureHistory("test_feature", result);
    
    std::vector<std::string> history = service.getFeatureHistory("test_feature");
    ASSERT_FALSE(history.empty());
}
