#include "AnnotationService.h"

namespace cad {
namespace drawings {

std::vector<Annotation> AnnotationService::buildDefaultAnnotations(const std::string& sheet) const {
    std::vector<Annotation> annotations;
    if (!sheet.empty()) {
        annotations.push_back(createTextAnnotation("Sheet note", 10.0, 10.0));
        annotations.push_back(createRevisionAnnotation("A", 10.0, 20.0));
        annotations.push_back(createTextAnnotation("Scale 1:1", 10.0, 30.0));
        annotations.push_back(createTextAnnotation("Parts list", 10.0, 40.0));
    }
    return annotations;
}

std::vector<Dimension> AnnotationService::buildDefaultDimensions(const std::string& sheet) const {
    std::vector<Dimension> dimensions;
    if (!sheet.empty()) {
        dimensions.push_back({"Width", 100.0, "mm", "+/-0.1"});
        dimensions.push_back({"Height", 50.0, "mm", "+/-0.1"});
        dimensions.push_back({"Depth", 25.0, "mm", "+/-0.1"});
    }
    return dimensions;
}

Annotation AnnotationService::createTextAnnotation(const std::string& text, double x, double y) const {
    return createAnnotation(AnnotationType::Text, text, x, y);
}

Annotation AnnotationService::createNoteAnnotation(const std::string& text, double x, double y, const std::string& view_name) const {
    Annotation ann = createAnnotation(AnnotationType::Note, text, x, y);
    ann.view_name = view_name;
    ann.attached_to_view = !view_name.empty();
    ann.font_size = 2.0;
    return ann;
}

Annotation AnnotationService::createCalloutAnnotation(const std::string& text, double x, double y, double leader_x, double leader_y) const {
    Annotation ann = createAnnotation(AnnotationType::Callout, text, x, y);
    LeaderPoint leader_point;
    leader_point.x = leader_x;
    leader_point.y = leader_y;
    ann.leader_points.push_back(leader_point);
    ann.has_leader = true;
    ann.font_size = 2.5;
    return ann;
}

Annotation AnnotationService::createLeaderAnnotation(const std::string& text, const std::vector<LeaderPoint>& leader_points, double text_x, double text_y) const {
    Annotation ann = createAnnotation(AnnotationType::Leader, text, text_x, text_y);
    ann.leader_points = leader_points;
    ann.has_leader = !leader_points.empty();
    ann.font_size = 2.5;
    return ann;
}

Annotation AnnotationService::createAttachedAnnotation(const std::string& text, const AttachmentPoint& attachment, double offset_x, double offset_y) const {
    Annotation ann = createAnnotation(AnnotationType::Note, text, attachment.x + offset_x, attachment.y + offset_y);
    ann.attachment_point = attachment;
    ann.has_attachment = true;
    ann.attached_to_view = true;
    
    // Create leader from attachment point to annotation text
    LeaderPoint leader_start;
    leader_start.x = attachment.x;
    leader_start.y = attachment.y;
    ann.leader_points.push_back(leader_start);
    
    LeaderPoint leader_end;
    leader_end.x = attachment.x + offset_x;
    leader_end.y = attachment.y + offset_y;
    ann.leader_points.push_back(leader_end);
    ann.has_leader = true;
    
    ann.font_size = 2.0;
    return ann;
}

Annotation AnnotationService::createBalloonAnnotation(const std::string& text, double x, double y, int balloon_number) const {
    Annotation ann = createAnnotation(AnnotationType::Balloon, text.empty() ? std::to_string(balloon_number) : text, x, y);
    ann.font_size = 3.0;
    ann.alignment = TextAlignment::Center;
    return ann;
}

Annotation AnnotationService::createRevisionAnnotation(const std::string& revision, double x, double y) const {
    Annotation ann = createAnnotation(AnnotationType::Revision, "REV " + revision, x, y);
    ann.font_size = 2.0;
    ann.alignment = TextAlignment::Left;
    return ann;
}

Annotation AnnotationService::createTitleAnnotation(const std::string& title, double x, double y) const {
    Annotation ann = createAnnotation(AnnotationType::Title, title, x, y);
    ann.font_size = 4.0;
    ann.alignment = TextAlignment::Center;
    ann.style_name = "Title";
    return ann;
}

std::vector<Annotation> AnnotationService::buildViewAnnotations(const std::string& view_name, const std::string& view_type) const {
    std::vector<Annotation> annotations;
    if (!view_name.empty()) {
        Annotation view_label = createTextAnnotation(view_name + " View", 50.0, 50.0);
        view_label.view_name = view_name;
        view_label.attached_to_view = true;
        view_label.alignment = TextAlignment::Center;
        annotations.push_back(view_label);
        
        if (view_type == "Isometric") {
            Annotation iso_note = createNoteAnnotation("ISO view", 50.0, 60.0, view_name);
            annotations.push_back(iso_note);
        }
    }
    return annotations;
}

std::vector<Annotation> AnnotationService::buildSheetAnnotations(const DrawingSheet& sheet) const {
    std::vector<Annotation> annotations;
    
    // Title block annotations
    annotations.push_back(createTitleAnnotation(sheet.name, 100.0, 10.0));
    annotations.push_back(createTextAnnotation("Scale: " + sheet.scale_label, 10.0, 10.0));
    
    // View annotations
    for (const auto& view : sheet.views) {
        auto view_anns = buildViewAnnotations(view.name, view.orientation);
        annotations.insert(annotations.end(), view_anns.begin(), view_anns.end());
    }
    
    return annotations;
}

LeaderPoint AnnotationService::createLeaderPoint(double x, double y) const {
    LeaderPoint point;
    point.x = x;
    point.y = y;
    return point;
}

AttachmentPoint AnnotationService::createAttachmentPoint(double x, double y, const std::string& entity_id) const {
    AttachmentPoint point;
    point.x = x;
    point.y = y;
    point.entity_id = entity_id;
    point.snap_to_geometry = !entity_id.empty();
    return point;
}

std::vector<LeaderPoint> AnnotationService::createPolylineLeader(const std::vector<std::pair<double, double>>& points) const {
    std::vector<LeaderPoint> leader_points;
    for (const auto& pt : points) {
        LeaderPoint leader_pt = createLeaderPoint(pt.first, pt.second);
        leader_points.push_back(leader_pt);
    }
    return leader_points;
}

Annotation AnnotationService::attachToGeometry(const Annotation& annotation, const AttachmentPoint& attachment) const {
    Annotation attached = annotation;
    attached.attachment_point = attachment;
    attached.has_attachment = true;
    
    // Add leader from attachment to annotation if not already present
    if (!attached.has_leader) {
        LeaderPoint start = createLeaderPoint(attachment.x, attachment.y);
        LeaderPoint end = createLeaderPoint(attached.x, attached.y);
        attached.leader_points.push_back(start);
        attached.leader_points.push_back(end);
        attached.has_leader = true;
    }
    
    return attached;
}

Annotation AnnotationService::createAnnotation(AnnotationType type, const std::string& text, double x, double y) const {
    Annotation ann;
    ann.text = text;
    ann.x = x;
    ann.y = y;
    ann.type = type;
    ann.alignment = TextAlignment::Left;
    ann.rotation = 0.0;
    ann.style_name = "Annotation";
    ann.font_size = 2.5;
    ann.attached_to_view = false;
    ann.has_leader = false;
    ann.has_attachment = false;
    return ann;
}

}  // namespace drawings
}  // namespace cad
