#pragma once

#include <string>
#include <vector>

#include "model/DrawingDocument.h"

namespace cad {
namespace drawings {

class AnnotationService {
public:
    std::vector<Annotation> buildDefaultAnnotations(const std::string& sheet) const;
    std::vector<Dimension> buildDefaultDimensions(const std::string& sheet) const;
    
    Annotation createTextAnnotation(const std::string& text, double x, double y) const;
    Annotation createNoteAnnotation(const std::string& text, double x, double y, const std::string& view_name = "") const;
    Annotation createCalloutAnnotation(const std::string& text, double x, double y, double leader_x, double leader_y) const;
    Annotation createLeaderAnnotation(const std::string& text, const std::vector<LeaderPoint>& leader_points, double text_x, double text_y) const;
    Annotation createAttachedAnnotation(const std::string& text, const AttachmentPoint& attachment, double offset_x, double offset_y) const;
    Annotation createBalloonAnnotation(const std::string& text, double x, double y, int balloon_number) const;
    Annotation createRevisionAnnotation(const std::string& revision, double x, double y) const;
    Annotation createTitleAnnotation(const std::string& title, double x, double y) const;
    
    std::vector<Annotation> buildViewAnnotations(const std::string& view_name, const std::string& view_type) const;
    std::vector<Annotation> buildSheetAnnotations(const DrawingSheet& sheet) const;
    
    LeaderPoint createLeaderPoint(double x, double y) const;
    AttachmentPoint createAttachmentPoint(double x, double y, const std::string& entity_id = "") const;
    std::vector<LeaderPoint> createPolylineLeader(const std::vector<std::pair<double, double>>& points) const;
    Annotation attachToGeometry(const Annotation& annotation, const AttachmentPoint& attachment) const;
    
private:
    Annotation createAnnotation(AnnotationType type, const std::string& text, double x, double y) const;
};

}  // namespace drawings
}  // namespace cad
