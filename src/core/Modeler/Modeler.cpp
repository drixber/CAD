#include "Modeler.h"

#include <utility>
#include <cctype>
#include <cstdlib>
#include <set>
#include <unordered_map>
#include <algorithm>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace cad {
namespace core {

Sketch::Sketch(std::string name) : name_(std::move(name)) {}

const std::string& Sketch::name() const {
    return name_;
}

void Sketch::addConstraint(const Constraint& constraint) {
    constraints_.push_back(constraint);
}

const std::vector<Constraint>& Sketch::constraints() const {
    return constraints_;
}

void Sketch::addParameter(const Parameter& parameter) {
    parameters_.push_back(parameter);
}

const std::vector<Parameter>& Sketch::parameters() const {
    return parameters_;
}

std::vector<Parameter>& Sketch::parameters() {
    return parameters_;
}

std::string Sketch::generateGeometryId() {
    return "geom_" + std::to_string(next_geometry_id_++);
}

std::string Sketch::addLine(const Point2D& start, const Point2D& end) {
    GeometryEntity entity;
    entity.id = generateGeometryId();
    entity.type = GeometryType::Line;
    entity.start_point = start;
    entity.end_point = end;
    geometry_.push_back(entity);
    return entity.id;
}

std::string Sketch::addCircle(const Point2D& center, double radius) {
    GeometryEntity entity;
    entity.id = generateGeometryId();
    entity.type = GeometryType::Circle;
    entity.center_point = center;
    entity.radius = radius;
    geometry_.push_back(entity);
    return entity.id;
}

std::string Sketch::addArc(const Point2D& center, double radius, double start_angle, double end_angle) {
    GeometryEntity entity;
    entity.id = generateGeometryId();
    entity.type = GeometryType::Arc;
    entity.center_point = center;
    entity.radius = radius;
    entity.start_angle = start_angle;
    entity.end_angle = end_angle;
    geometry_.push_back(entity);
    return entity.id;
}

std::string Sketch::addRectangle(const Point2D& corner, double width, double height) {
    GeometryEntity entity;
    entity.id = generateGeometryId();
    entity.type = GeometryType::Rectangle;
    entity.start_point = corner;  // Use start_point as corner
    entity.width = width;
    entity.height = height;
    geometry_.push_back(entity);
    return entity.id;
}

std::string Sketch::addPoint(const Point2D& point) {
    GeometryEntity entity;
    entity.id = generateGeometryId();
    entity.type = GeometryType::Point;
    entity.start_point = point;
    geometry_.push_back(entity);
    return entity.id;
}

std::string Sketch::addEllipse(const Point2D& center, double radius_major, double radius_minor) {
    GeometryEntity entity;
    entity.id = generateGeometryId();
    entity.type = GeometryType::Ellipse;
    entity.center_point = center;
    entity.radius = radius_major;
    entity.width = radius_minor;
    geometry_.push_back(entity);
    return entity.id;
}

std::string Sketch::addPolygon(const std::vector<Point2D>& points) {
    if (points.empty()) {
        return {};
    }
    GeometryEntity entity;
    entity.id = generateGeometryId();
    entity.type = GeometryType::Polygon;
    entity.start_point = points.front();
    entity.end_point = points.size() > 1 ? points.back() : points.front();
    geometry_.push_back(entity);
    return entity.id;
}

std::string Sketch::addSpline(const std::vector<Point2D>& control_points) {
    if (control_points.empty()) {
        return {};
    }
    GeometryEntity entity;
    entity.id = generateGeometryId();
    entity.type = GeometryType::Spline;
    entity.start_point = control_points.front();
    entity.end_point = control_points.size() > 1 ? control_points.back() : control_points.front();
    geometry_.push_back(entity);
    return entity.id;
}

std::string Sketch::addText(const Point2D& position, const std::string& text) {
    GeometryEntity entity;
    entity.id = generateGeometryId();
    entity.type = GeometryType::Text;
    entity.start_point = position;
    entity.text_content = text;
    geometry_.push_back(entity);
    return entity.id;
}

const std::vector<GeometryEntity>& Sketch::geometry() const {
    return geometry_;
}

GeometryEntity* Sketch::findGeometry(const std::string& id) {
    for (auto& entity : geometry_) {
        if (entity.id == id) {
            return &entity;
        }
    }
    return nullptr;
}

const GeometryEntity* Sketch::findGeometry(const std::string& id) const {
    for (const auto& entity : geometry_) {
        if (entity.id == id) {
            return &entity;
        }
    }
    return nullptr;
}

bool Sketch::removeGeometry(const std::string& id) {
    auto it = std::remove_if(geometry_.begin(), geometry_.end(),
        [&id](const GeometryEntity& entity) { return entity.id == id; });
    if (it != geometry_.end()) {
        geometry_.erase(it, geometry_.end());
        return true;
    }
    return false;
}

void Sketch::set3D(bool is_3d) {
    is_3d_ = is_3d;
}

bool Sketch::is3D() const {
    return is_3d_;
}

std::string Sketch::addWaypoint3D(double x, double y, double z) {
    waypoints_3d_.push_back(Point3D{x, y, z});
    return "WP3D_" + std::to_string(waypoints_3d_.size());
}

const std::vector<Point3D>& Sketch::waypoints3D() const {
    return waypoints_3d_;
}

Part::Part(std::string name) : name_(std::move(name)) {}

const std::string& Part::name() const {
    return name_;
}

void Part::addFeature(const Feature& feature) {
    features_.push_back(feature);
}

const std::vector<Feature>& Part::features() const {
    return features_;
}

std::string Part::generateFeatureName(FeatureType type) {
    std::string prefix;
    switch (type) {
        case FeatureType::Extrude:
            prefix = "Extrude";
            break;
        case FeatureType::Revolve:
            prefix = "Revolve";
            break;
        case FeatureType::Loft:
            prefix = "Loft";
            break;
        case FeatureType::Hole:
            prefix = "Hole";
            break;
        case FeatureType::Fillet:
            prefix = "Fillet";
            break;
        case FeatureType::Chamfer:
            prefix = "Chamfer";
            break;
        case FeatureType::Shell:
            prefix = "Shell";
            break;
        case FeatureType::Pattern:
            prefix = "Pattern";
            break;
        case FeatureType::CircularPattern:
            prefix = "CircularPattern";
            break;
        case FeatureType::PathPattern:
            prefix = "PathPattern";
            break;
        case FeatureType::Sweep:
            prefix = "Sweep";
            break;
        case FeatureType::Helix:
            prefix = "Helix";
            break;
        case FeatureType::Draft:
            prefix = "Draft";
            break;
        case FeatureType::Mirror:
            prefix = "Mirror";
            break;
        case FeatureType::Thread:
            prefix = "Thread";
            break;
        case FeatureType::Rib:
            prefix = "Rib";
            break;
    }
    return prefix + std::to_string(next_feature_id_++);
}

std::string Part::createExtrude(const std::string& sketch_id, double depth, bool symmetric, ExtrudeMode mode) {
    Feature feature;
    feature.name = generateFeatureName(FeatureType::Extrude);
    feature.type = FeatureType::Extrude;
    feature.sketch_id = sketch_id;
    feature.depth = depth;
    feature.symmetric = symmetric;
    feature.extrude_mode = mode;
    feature.parameters["extrude_mode"] = static_cast<double>(static_cast<int>(mode));
    features_.push_back(feature);
    return feature.name;
}

std::string Part::createRevolve(const std::string& sketch_id, double angle, const std::string& axis) {
    Feature feature;
    feature.name = generateFeatureName(FeatureType::Revolve);
    feature.type = FeatureType::Revolve;
    feature.sketch_id = sketch_id;
    feature.angle = angle;
    feature.axis = axis;
    features_.push_back(feature);
    return feature.name;
}

std::string Part::createLoft(const std::vector<std::string>& sketch_ids) {
    Feature feature;
    feature.name = generateFeatureName(FeatureType::Loft);
    feature.type = FeatureType::Loft;
    if (!sketch_ids.empty()) {
        feature.sketch_id = sketch_ids[0];  // Primary sketch
        // Store additional sketches in parameters
        for (size_t i = 1; i < sketch_ids.size(); ++i) {
            feature.parameters["sketch_" + std::to_string(i)] = static_cast<double>(i);
        }
    }
    features_.push_back(feature);
    return feature.name;
}

std::string Part::createHole(double diameter, double depth, bool through_all) {
    Feature feature;
    feature.name = generateFeatureName(FeatureType::Hole);
    feature.type = FeatureType::Hole;
    feature.diameter = diameter;
    feature.hole_depth = depth;
    feature.through_all = through_all;
    features_.push_back(feature);
    return feature.name;
}

std::string Part::createFillet(double radius, const std::vector<std::string>& edge_ids) {
    Feature feature;
    feature.name = generateFeatureName(FeatureType::Fillet);
    feature.type = FeatureType::Fillet;
    feature.radius = radius;
    feature.edge_ids = edge_ids;
    features_.push_back(feature);
    return feature.name;
}

std::string Part::createPattern(const std::string& base_feature, int count_x, int count_y, int count_z,
                                double spacing_x, double spacing_y, double spacing_z) {
    Feature feature;
    feature.name = generateFeatureName(FeatureType::Pattern);
    feature.type = FeatureType::Pattern;
    feature.sketch_id = base_feature;  // Reuse sketch_id field for base feature
    feature.count_x = count_x;
    feature.count_y = count_y;
    feature.count_z = count_z;
    feature.spacing_x = spacing_x;
    feature.spacing_y = spacing_y;
    feature.spacing_z = spacing_z;
    features_.push_back(feature);
    return feature.name;
}

std::string Part::createCircularPattern(const std::string& base_feature, int count, double angle_deg,
                                        const std::string& axis) {
    Feature feature;
    feature.name = generateFeatureName(FeatureType::CircularPattern);
    feature.type = FeatureType::CircularPattern;
    feature.sketch_id = base_feature;
    feature.circular_count = count > 0 ? count : 4;
    feature.circular_angle = angle_deg;
    feature.circular_axis = axis.empty() ? "Z" : axis;
    features_.push_back(feature);
    return feature.name;
}

std::string Part::createPathPattern(const std::string& base_feature, const std::string& path_sketch_id,
                                     int count, bool equal_spacing) {
    Feature feature;
    feature.name = generateFeatureName(FeatureType::PathPattern);
    feature.type = FeatureType::PathPattern;
    feature.sketch_id = base_feature;
    feature.path_sketch_id = path_sketch_id;
    feature.path_count = count > 0 ? count : 4;
    feature.path_equal_spacing = equal_spacing;
    features_.push_back(feature);
    return feature.name;
}

std::string Part::createThinExtrude(const std::string& sketch_id, double depth, double wall_thickness,
                                    bool symmetric, ExtrudeMode mode) {
    Feature feature;
    feature.name = generateFeatureName(FeatureType::Extrude);
    feature.type = FeatureType::Extrude;
    feature.sketch_id = sketch_id;
    feature.depth = depth;
    feature.symmetric = symmetric;
    feature.extrude_mode = mode;
    feature.thin_wall = true;
    feature.thin_thickness = wall_thickness > 0 ? wall_thickness : 1.0;
    feature.parameters["extrude_mode"] = static_cast<double>(static_cast<int>(mode));
    features_.push_back(feature);
    return feature.name;
}

std::string Part::createSweep(const std::string& profile_sketch_id, const std::string& path_sketch_id,
                             double twist_angle, double scale_factor) {
    Feature feature;
    feature.name = generateFeatureName(FeatureType::Sweep);
    feature.type = FeatureType::Sweep;
    feature.sketch_id = profile_sketch_id;
    feature.path_sketch_id = path_sketch_id;
    feature.twist_angle = twist_angle;
    feature.scale_factor = scale_factor;
    features_.push_back(feature);
    return feature.name;
}

std::string Part::createHelix(double radius, double pitch, double revolutions, bool clockwise) {
    Feature feature;
    feature.name = generateFeatureName(FeatureType::Helix);
    feature.type = FeatureType::Helix;
    feature.parameters["radius"] = radius;
    feature.pitch = pitch;
    feature.revolutions = revolutions;
    feature.clockwise = clockwise;
    features_.push_back(feature);
    return feature.name;
}

std::string Part::createChamfer(double distance1, double distance2, double angle,
                               const std::vector<std::string>& edge_ids) {
    Feature feature;
    feature.name = generateFeatureName(FeatureType::Chamfer);
    feature.type = FeatureType::Chamfer;
    feature.parameters["distance1"] = distance1;
    feature.parameters["distance2"] = distance2;
    feature.parameters["angle"] = angle;
    feature.edge_ids = edge_ids;
    features_.push_back(feature);
    return feature.name;
}

std::string Part::createShell(double wall_thickness, const std::vector<std::string>& face_ids) {
    Feature feature;
    feature.name = generateFeatureName(FeatureType::Shell);
    feature.type = FeatureType::Shell;
    feature.wall_thickness = wall_thickness;
    feature.face_ids = face_ids;
    features_.push_back(feature);
    return feature.name;
}

std::string Part::createDraft(double angle, const std::string& draft_plane,
                            const std::vector<std::string>& face_ids) {
    Feature feature;
    feature.name = generateFeatureName(FeatureType::Draft);
    feature.type = FeatureType::Draft;
    feature.draft_angle = angle;
    feature.draft_plane = draft_plane;
    feature.face_ids = face_ids;
    features_.push_back(feature);
    return feature.name;
}

std::string Part::createMirror(const std::string& base_feature, const std::string& mirror_plane,
                              bool merge_result) {
    Feature feature;
    feature.name = generateFeatureName(FeatureType::Mirror);
    feature.type = FeatureType::Mirror;
    feature.sketch_id = base_feature;
    feature.mirror_plane = mirror_plane;
    feature.merge_result = merge_result;
    features_.push_back(feature);
    return feature.name;
}

std::string Part::createThread(const std::string& thread_standard, double pitch, bool internal) {
    Feature feature;
    feature.name = generateFeatureName(FeatureType::Thread);
    feature.type = FeatureType::Thread;
    feature.thread_standard = thread_standard;
    feature.thread_pitch = pitch;
    feature.internal = internal;
    features_.push_back(feature);
    return feature.name;
}

std::string Part::createRib(double thickness, const std::string& rib_plane, const std::string& sketch_id) {
    Feature feature;
    feature.name = generateFeatureName(FeatureType::Rib);
    feature.type = FeatureType::Rib;
    feature.rib_thickness = thickness;
    feature.rib_plane = rib_plane;
    feature.sketch_id = sketch_id;
    features_.push_back(feature);
    return feature.name;
}

Feature* Part::findFeature(const std::string& name) {
    for (auto& feature : features_) {
        if (feature.name == name) {
            return &feature;
        }
    }
    return nullptr;
}

const Feature* Part::findFeature(const std::string& name) const {
    for (const auto& feature : features_) {
        if (feature.name == name) {
            return &feature;
        }
    }
    return nullptr;
}

bool Part::removeFeature(const std::string& name) {
    auto it = std::remove_if(features_.begin(), features_.end(),
        [&name](const Feature& feature) { return feature.name == name; });
    if (it != features_.end()) {
        features_.erase(it, features_.end());
        return true;
    }
    return false;
}

std::string Part::generateReferenceId(const char* prefix) {
    return std::string(prefix) + std::to_string(next_ref_id_++);
}

std::string Part::addWorkPlane(const std::string& name, const Point3D& origin, const Vector3D& normal) {
    WorkPlane wp;
    wp.id = generateReferenceId("Plane");
    wp.name = name.empty() ? wp.id : name;
    wp.origin = origin;
    wp.normal = normal;
    work_planes_.push_back(wp);
    return wp.id;
}

std::string Part::addWorkPlaneOffset(const std::string& name, const std::string& base_plane, double offset) {
    WorkPlane wp;
    wp.id = generateReferenceId("Plane");
    wp.name = name.empty() ? wp.id : name;
    wp.base_plane = base_plane;
    wp.offset = offset;
    if (base_plane == "XY") {
        wp.origin = Point3D{0, 0, offset};
        wp.normal = Vector3D{0, 0, 1};
    } else if (base_plane == "YZ") {
        wp.origin = Point3D{offset, 0, 0};
        wp.normal = Vector3D{1, 0, 0};
    } else if (base_plane == "XZ") {
        wp.origin = Point3D{0, offset, 0};
        wp.normal = Vector3D{0, 1, 0};
    } else {
        wp.origin = Point3D{0, 0, offset};
        wp.normal = Vector3D{0, 0, 1};
    }
    work_planes_.push_back(wp);
    return wp.id;
}

std::string Part::addWorkAxis(const std::string& name, const Point3D& point, const Vector3D& direction) {
    WorkAxis wa;
    wa.id = generateReferenceId("Axis");
    wa.name = name.empty() ? wa.id : name;
    wa.point = point;
    wa.direction = direction;
    work_axes_.push_back(wa);
    return wa.id;
}

std::string Part::addWorkAxisBase(const std::string& name, const std::string& base_axis) {
    WorkAxis wa;
    wa.id = generateReferenceId("Axis");
    wa.name = name.empty() ? wa.id : name;
    wa.base_axis = base_axis;
    wa.point = Point3D{0, 0, 0};
    if (base_axis == "X") wa.direction = Vector3D{1, 0, 0};
    else if (base_axis == "Y") wa.direction = Vector3D{0, 1, 0};
    else wa.direction = Vector3D{0, 0, 1};
    work_axes_.push_back(wa);
    return wa.id;
}

std::string Part::addWorkPoint(const std::string& name, const Point3D& point) {
    WorkPoint wp;
    wp.id = generateReferenceId("Point");
    wp.name = name.empty() ? wp.id : name;
    wp.point = point;
    work_points_.push_back(wp);
    return wp.id;
}

std::string Part::addCoordinateSystem(const std::string& name, const Point3D& origin,
                                      const Vector3D& dir_x, const Vector3D& dir_y) {
    CoordinateSystem cs;
    cs.id = generateReferenceId("CS");
    cs.name = name.empty() ? cs.id : name;
    cs.origin = origin;
    cs.direction_x = dir_x;
    cs.direction_y = dir_y;
    coordinate_systems_.push_back(cs);
    return cs.id;
}

const std::vector<WorkPlane>& Part::workPlanes() const {
    return work_planes_;
}

const std::vector<WorkAxis>& Part::workAxes() const {
    return work_axes_;
}

const std::vector<WorkPoint>& Part::workPoints() const {
    return work_points_;
}

const std::vector<CoordinateSystem>& Part::coordinateSystems() const {
    return coordinate_systems_;
}

WorkPlane* Part::findWorkPlane(const std::string& id) {
    for (auto& wp : work_planes_) {
        if (wp.id == id) return &wp;
    }
    return nullptr;
}

WorkAxis* Part::findWorkAxis(const std::string& id) {
    for (auto& wa : work_axes_) {
        if (wa.id == id) return &wa;
    }
    return nullptr;
}

WorkPoint* Part::findWorkPoint(const std::string& id) {
    for (auto& wp : work_points_) {
        if (wp.id == id) return &wp;
    }
    return nullptr;
}

void Part::addUserParameter(const Parameter& p) {
    user_parameters_.push_back(p);
}

std::vector<Parameter>& Part::userParameters() {
    return user_parameters_;
}

const std::vector<Parameter>& Part::userParameters() const {
    return user_parameters_;
}

bool Part::setParameterValue(const std::string& name, double value) {
    Parameter* p = findParameter(name);
    if (!p) return false;
    p->value = value;
    return true;
}

bool Part::removeParameter(const std::string& name) {
    auto it = std::find_if(user_parameters_.begin(), user_parameters_.end(),
        [&name](const Parameter& p) { return p.name == name; });
    if (it == user_parameters_.end()) return false;
    user_parameters_.erase(it);
    return true;
}

Parameter* Part::findParameter(const std::string& name) {
    for (auto& p : user_parameters_) {
        if (p.name == name) return &p;
    }
    return nullptr;
}

const Parameter* Part::findParameter(const std::string& name) const {
    for (const auto& p : user_parameters_) {
        if (p.name == name) return &p;
    }
    return nullptr;
}

void Part::addRule(const Rule& rule) {
    rules_.push_back(rule);
}

std::vector<Rule>& Part::rules() {
    return rules_;
}

const std::vector<Rule>& Part::rules() const {
    return rules_;
}

void Part::addConfiguration(const Configuration& config) {
    configurations_.push_back(config);
}

const std::vector<Configuration>& Part::configurations() const {
    return configurations_;
}

void Part::setActiveConfiguration(int index) {
    if (index >= 0 && index < static_cast<int>(configurations_.size()))
        active_configuration_index_ = index;
}

int Part::activeConfigurationIndex() const {
    return active_configuration_index_;
}

void Part::setSkeletonPartId(const std::string& part_id) {
    skeleton_part_id_ = part_id;
}

std::string Part::skeletonPartId() const {
    return skeleton_part_id_;
}

void Part::setRollbackPosition(int feature_index) {
    rollback_position_ = feature_index;
}

int Part::rollbackPosition() const {
    return rollback_position_;
}

bool Part::setFeatureSuppressed(const std::string& feature_name, bool suppressed) {
    for (auto& f : features_) {
        if (f.name == feature_name) {
            f.suppressed = suppressed;
            return true;
        }
    }
    return false;
}

bool Part::isFeatureSuppressed(const std::string& feature_name) const {
    for (const auto& f : features_) {
        if (f.name == feature_name) return f.suppressed;
    }
    return false;
}

bool Part::reorderFeature(std::size_t from_index, std::size_t to_index) {
    if (from_index >= features_.size() || to_index >= features_.size() || from_index == to_index)
        return false;
    Feature f = features_[from_index];
    features_.erase(features_.begin() + static_cast<std::ptrdiff_t>(from_index));
    features_.insert(features_.begin() + static_cast<std::ptrdiff_t>(to_index), f);
    return true;
}

std::uint64_t Assembly::addComponent(const Part& part, const Transform& transform) {
    AssemblyComponent component;
    component.id = next_id_++;
    component.part = part;
    component.transform = transform;
    components_.push_back(component);
    return component.id;
}

const std::vector<AssemblyComponent>& Assembly::components() const {
    return components_;
}

void Assembly::addMate(const MateConstraint& mate) {
    mates_.push_back(mate);
}

const std::vector<MateConstraint>& Assembly::mates() const {
    return mates_;
}

std::string Assembly::createMate(std::uint64_t component_a, std::uint64_t component_b, double offset) {
    MateConstraint mate;
    mate.component_a = component_a;
    mate.component_b = component_b;
    mate.type = MateType::Mate;
    mate.value = offset;
    mates_.push_back(mate);
    return "Mate_" + std::to_string(mates_.size());
}

std::string Assembly::createFlush(std::uint64_t component_a, std::uint64_t component_b, double offset) {
    MateConstraint mate;
    mate.component_a = component_a;
    mate.component_b = component_b;
    mate.type = MateType::Flush;
    mate.value = offset;
    mates_.push_back(mate);
    return "Flush_" + std::to_string(mates_.size());
}

std::string Assembly::createConcentric(std::uint64_t component_a, std::uint64_t component_b, double offset) {
    MateConstraint mate;
    mate.component_a = component_a;
    mate.component_b = component_b;
    mate.type = MateType::Concentric;
    mate.value = offset;
    mates_.push_back(mate);
    return "Concentric_" + std::to_string(mates_.size());
}

std::string Assembly::createTangent(std::uint64_t component_a, std::uint64_t component_b, double offset) {
    MateConstraint mate;
    mate.component_a = component_a;
    mate.component_b = component_b;
    mate.type = MateType::Tangent;
    mate.value = offset;
    mates_.push_back(mate);
    return "Tangent_" + std::to_string(mates_.size());
}

std::string Assembly::createParallel(std::uint64_t component_a, std::uint64_t component_b, double distance) {
    MateConstraint mate;
    mate.component_a = component_a;
    mate.component_b = component_b;
    mate.type = MateType::Parallel;
    mate.value = distance;
    mates_.push_back(mate);
    return "Parallel_" + std::to_string(mates_.size());
}

std::string Assembly::createDistance(std::uint64_t component_a, std::uint64_t component_b, double distance) {
    MateConstraint mate;
    mate.component_a = component_a;
    mate.component_b = component_b;
    mate.type = MateType::Distance;
    mate.value = distance;
    mates_.push_back(mate);
    return "Distance_" + std::to_string(mates_.size());
}

std::string Assembly::createAngle(std::uint64_t component_a, std::uint64_t component_b, double angle) {
    MateConstraint mate;
    mate.component_a = component_a;
    mate.component_b = component_b;
    mate.type = MateType::Angle;
    mate.value = angle;
    mates_.push_back(mate);
    return "Angle_" + std::to_string(mates_.size());
}

std::string Assembly::createInsert(std::uint64_t component_a, std::uint64_t component_b) {
    MateConstraint mate;
    mate.component_a = component_a;
    mate.component_b = component_b;
    mate.type = MateType::Insert;
    mate.value = 0.0;
    mates_.push_back(mate);
    return "Insert_" + std::to_string(mates_.size());
}

std::string Assembly::createGear(std::uint64_t component_a, std::uint64_t component_b, double ratio) {
    MateConstraint mate;
    mate.component_a = component_a;
    mate.component_b = component_b;
    mate.type = MateType::Gear;
    mate.value = ratio;
    mates_.push_back(mate);
    return "Gear_" + std::to_string(mates_.size());
}

std::string Assembly::createCam(std::uint64_t component_a, std::uint64_t component_b, double phase_offset) {
    MateConstraint mate;
    mate.component_a = component_a;
    mate.component_b = component_b;
    mate.type = MateType::Cam;
    mate.value = phase_offset;
    mates_.push_back(mate);
    return "Cam_" + std::to_string(mates_.size());
}

bool Assembly::solveMates() {
    const int max_iterations = 10;
    const double tolerance = 0.001;
    
    for (int iteration = 0; iteration < max_iterations; ++iteration) {
        bool converged = true;
        
        for (const auto& mate : mates_) {
            AssemblyComponent* comp_a = findComponent(mate.component_a);
            AssemblyComponent* comp_b = findComponent(mate.component_b);
            
            if (!comp_a || !comp_b) {
                continue;
            }
            
            double error = 0.0;
            
            switch (mate.type) {
                case MateType::Mate: {
                    double dx = comp_b->transform.tx - (comp_a->transform.tx + mate.value);
                    double dy = comp_b->transform.ty - comp_a->transform.ty;
                    double dz = comp_b->transform.tz - comp_a->transform.tz;
                    error = std::sqrt(dx*dx + dy*dy + dz*dz);
                    
                    if (error > tolerance) {
                        comp_b->transform.tx = comp_a->transform.tx + mate.value;
                        comp_b->transform.ty = comp_a->transform.ty;
                        comp_b->transform.tz = comp_a->transform.tz;
                        converged = false;
                    }
                    break;
                }
                case MateType::Flush: {
                    double dx = comp_b->transform.tx - (comp_a->transform.tx + mate.value);
                    error = std::abs(dx);
                    
                    if (error > tolerance) {
                        comp_b->transform.tx = comp_a->transform.tx + mate.value;
                        converged = false;
                    }
                    break;
                }
                case MateType::Angle: {
                    double angle_diff = comp_b->transform.rz - (comp_a->transform.rz + mate.value);
                    while (angle_diff > M_PI) angle_diff -= 2.0 * M_PI;
                    while (angle_diff < -M_PI) angle_diff += 2.0 * M_PI;
                    error = std::abs(angle_diff);
                    
                    if (error > tolerance) {
                        comp_b->transform.rz = comp_a->transform.rz + mate.value;
                        converged = false;
                    }
                    break;
                }
                case MateType::Insert: {
                    double dx = comp_b->transform.tx - comp_a->transform.tx;
                    double dy = comp_b->transform.ty - comp_a->transform.ty;
                    double dz = comp_b->transform.tz - comp_a->transform.tz;
                    error = std::sqrt(dx*dx + dy*dy + dz*dz);
                    
                    if (error > tolerance) {
                        comp_b->transform.tx = comp_a->transform.tx;
                        comp_b->transform.ty = comp_a->transform.ty;
                        comp_b->transform.tz = comp_a->transform.tz;
                        converged = false;
                    }
                    break;
                }
                case MateType::Concentric: {
                    double dx = comp_b->transform.tx - (comp_a->transform.tx + mate.value);
                    double dy = comp_b->transform.ty - comp_a->transform.ty;
                    double dz = comp_b->transform.tz - comp_a->transform.tz;
                    error = std::sqrt(dx*dx + dy*dy + dz*dz);
                    if (error > tolerance) {
                        comp_b->transform.tx = comp_a->transform.tx + mate.value;
                        comp_b->transform.ty = comp_a->transform.ty;
                        comp_b->transform.tz = comp_a->transform.tz;
                        converged = false;
                    }
                    break;
                }
                case MateType::Tangent: {
                    double dx = comp_b->transform.tx - (comp_a->transform.tx + mate.value);
                    error = std::abs(dx);
                    if (error > tolerance) {
                        comp_b->transform.tx = comp_a->transform.tx + mate.value;
                        converged = false;
                    }
                    break;
                }
                case MateType::Gear: {
                    // Kinematik: comp_b.rz = comp_a.rz * ratio (value = ratio)
                    double target_rz = comp_a->transform.rz * mate.value;
                    double angle_diff = comp_b->transform.rz - target_rz;
                    while (angle_diff > M_PI) angle_diff -= 2.0 * M_PI;
                    while (angle_diff < -M_PI) angle_diff += 2.0 * M_PI;
                    error = std::abs(angle_diff);
                    if (error > tolerance) {
                        comp_b->transform.rz = target_rz;
                        converged = false;
                    }
                    break;
                }
                case MateType::Cam: {
                    // Phasenversatz: comp_b.rz = comp_a.rz + mate.value
                    double target_rz = comp_a->transform.rz + mate.value;
                    double angle_diff = comp_b->transform.rz - target_rz;
                    while (angle_diff > M_PI) angle_diff -= 2.0 * M_PI;
                    while (angle_diff < -M_PI) angle_diff += 2.0 * M_PI;
                    error = std::abs(angle_diff);
                    if (error > tolerance) {
                        comp_b->transform.rz = target_rz;
                        converged = false;
                    }
                    break;
                }
                case MateType::Parallel: {
                    // Ebenen/Achsen parallel: angle alignment, value = optional distance
                    double angle_diff = comp_b->transform.rz - comp_a->transform.rz;
                    while (angle_diff > M_PI) angle_diff -= 2.0 * M_PI;
                    while (angle_diff < -M_PI) angle_diff += 2.0 * M_PI;
                    error = std::abs(angle_diff);
                    if (error > tolerance) {
                        comp_b->transform.rz = comp_a->transform.rz;
                        converged = false;
                    }
                    if (std::abs(mate.value) > 1e-9) {
                        double dx = comp_b->transform.tx - (comp_a->transform.tx + mate.value);
                        if (std::abs(dx) > tolerance) {
                            comp_b->transform.tx = comp_a->transform.tx + mate.value;
                            converged = false;
                        }
                    }
                    break;
                }
                case MateType::Distance: {
                    double dx = comp_b->transform.tx - comp_a->transform.tx;
                    double dy = comp_b->transform.ty - comp_a->transform.ty;
                    double dz = comp_b->transform.tz - comp_a->transform.tz;
                    double actual = std::sqrt(dx*dx + dy*dy + dz*dz);
                    error = std::abs(actual - mate.value);
                    if (error > tolerance && actual > 1e-9) {
                        double scale = mate.value / actual;
                        comp_b->transform.tx = comp_a->transform.tx + dx * scale;
                        comp_b->transform.ty = comp_a->transform.ty + dy * scale;
                        comp_b->transform.tz = comp_a->transform.tz + dz * scale;
                        converged = false;
                    }
                    break;
                }
            }
        }
        
        if (converged) {
            return true;
        }
    }
    
    return true;
}

AssemblyComponent* Assembly::findComponent(std::uint64_t id) {
    for (auto& component : components_) {
        if (component.id == id) {
            return &component;
        }
    }
    return nullptr;
}

const AssemblyComponent* Assembly::findComponent(std::uint64_t id) const {
    for (const auto& component : components_) {
        if (component.id == id) {
            return &component;
        }
    }
    return nullptr;
}

void Assembly::addJoint(const Joint& joint) {
    joints_.push_back(joint);
}

const std::vector<Joint>& Assembly::joints() const {
    return joints_;
}

int Assembly::getJointDegreesOfFreedom() const {
    int dof = 0;
    for (const auto& j : joints_) {
        switch (j.type) {
            case JointType::Rigid:
                break;
            case JointType::Revolute:
            case JointType::Slider:
                dof += 1;
                break;
            case JointType::Cylindrical:
                dof += 2;
                break;
            case JointType::Planar:
                dof += 3;
                break;
            case JointType::PinSlot:
                dof += 2;
                break;
        }
    }
    return dof;
}

std::string Assembly::createRevolute(std::uint64_t comp_a, std::uint64_t comp_b,
                                     double axis_x, double axis_y, double axis_z,
                                     double limit_lo, double limit_hi) {
    Joint j;
    j.component_a = comp_a;
    j.component_b = comp_b;
    j.type = JointType::Revolute;
    j.axis_direction.x = axis_x;
    j.axis_direction.y = axis_y;
    j.axis_direction.z = axis_z;
    j.limit_low = limit_lo;
    j.limit_high = limit_hi;
    joints_.push_back(j);
    return "Revolute_" + std::to_string(joints_.size());
}

std::string Assembly::createSlider(std::uint64_t comp_a, std::uint64_t comp_b,
                                   double axis_x, double axis_y, double axis_z,
                                   double limit_lo, double limit_hi) {
    Joint j;
    j.component_a = comp_a;
    j.component_b = comp_b;
    j.type = JointType::Slider;
    j.axis_direction.x = axis_x;
    j.axis_direction.y = axis_y;
    j.axis_direction.z = axis_z;
    j.limit_low = limit_lo;
    j.limit_high = limit_hi;
    joints_.push_back(j);
    return "Slider_" + std::to_string(joints_.size());
}

std::string Assembly::createCylindrical(std::uint64_t comp_a, std::uint64_t comp_b,
                                        double axis_x, double axis_y, double axis_z) {
    Joint j;
    j.component_a = comp_a;
    j.component_b = comp_b;
    j.type = JointType::Cylindrical;
    j.axis_direction.x = axis_x;
    j.axis_direction.y = axis_y;
    j.axis_direction.z = axis_z;
    joints_.push_back(j);
    return "Cylindrical_" + std::to_string(joints_.size());
}

std::string Assembly::createPlanar(std::uint64_t comp_a, std::uint64_t comp_b,
                                   double normal_x, double normal_y, double normal_z) {
    Joint j;
    j.component_a = comp_a;
    j.component_b = comp_b;
    j.type = JointType::Planar;
    j.axis_direction.x = normal_x;
    j.axis_direction.y = normal_y;
    j.axis_direction.z = normal_z;
    joints_.push_back(j);
    return "Planar_" + std::to_string(joints_.size());
}

std::string Assembly::createPinSlot(std::uint64_t comp_a, std::uint64_t comp_b,
                                    double axis_x, double axis_y, double axis_z,
                                    double slot_x, double slot_y, double slot_z,
                                    double limit_lo, double limit_hi) {
    Joint j;
    j.component_a = comp_a;
    j.component_b = comp_b;
    j.type = JointType::PinSlot;
    j.axis_direction.x = axis_x;
    j.axis_direction.y = axis_y;
    j.axis_direction.z = axis_z;
    j.slot_direction.x = slot_x;
    j.slot_direction.y = slot_y;
    j.slot_direction.z = slot_z;
    j.limit_low = limit_lo;
    j.limit_high = limit_hi;
    joints_.push_back(j);
    return "PinSlot_" + std::to_string(joints_.size());
}

void Assembly::setExplosionOffset(std::uint64_t component_id, double dx, double dy, double dz) {
    explosion_offsets_[component_id] = Vector3D{dx, dy, dz};
}

Vector3D Assembly::getExplosionOffset(std::uint64_t component_id) const {
    auto it = explosion_offsets_.find(component_id);
    if (it != explosion_offsets_.end()) {
        return it->second;
    }
    return Vector3D{0.0, 0.0, 0.0};
}

void Assembly::setExplosionFactor(double factor) {
    explosion_factor_ = std::max(0.0, std::min(1.0, factor));
}

double Assembly::getExplosionFactor() const {
    return explosion_factor_;
}

void Assembly::clearExplosionOffsets() {
    explosion_offsets_.clear();
}

bool Assembly::hasExplosionOffsets() const {
    return !explosion_offsets_.empty();
}

Transform Assembly::getDisplayTransform(std::uint64_t component_id) const {
    const AssemblyComponent* comp = findComponent(component_id);
    if (!comp) {
        return Transform{};
    }
    Transform out = comp->transform;
    Vector3D off = getExplosionOffset(component_id);
    out.tx += off.x * explosion_factor_;
    out.ty += off.y * explosion_factor_;
    out.tz += off.z * explosion_factor_;
    return out;
}

void Assembly::setComponentLightweight(std::uint64_t component_id, bool lightweight) {
    if (lightweight) {
        lightweight_components_.insert(component_id);
    } else {
        lightweight_components_.erase(component_id);
    }
    AssemblyComponent* comp = findComponent(component_id);
    if (comp) {
        comp->lightweight_display = lightweight;
    }
}

bool Assembly::isComponentLightweight(std::uint64_t component_id) const {
    return lightweight_components_.find(component_id) != lightweight_components_.end();
}

void Assembly::setComponentFlexible(std::uint64_t component_id, bool flexible) {
    if (flexible) {
        flexible_components_.insert(component_id);
    } else {
        flexible_components_.erase(component_id);
    }
    AssemblyComponent* comp = findComponent(component_id);
    if (comp) {
        comp->flexible_subassembly = flexible;
    }
}

bool Assembly::isComponentFlexible(std::uint64_t component_id) const {
    return flexible_components_.find(component_id) != flexible_components_.end();
}

void Assembly::addConfiguration(const AssemblyConfiguration& config) {
    configurations_.push_back(config);
}

const std::vector<AssemblyConfiguration>& Assembly::configurations() const {
    return configurations_;
}

void Assembly::setActiveConfiguration(int index) {
    if (index >= 0 && index < static_cast<int>(configurations_.size()))
        active_configuration_index_ = index;
}

int Assembly::activeConfigurationIndex() const {
    return active_configuration_index_;
}

void Assembly::addArrangement(const AssemblyConfiguration& config) {
    addConfiguration(config);
}

const std::vector<AssemblyConfiguration>& Assembly::arrangements() const {
    return configurations_;
}

void Assembly::addComponentInterface(std::uint64_t component_id, const std::string& interface_name) {
    component_interfaces_[component_id].push_back(interface_name);
}

std::vector<std::string> Assembly::getComponentInterfaces(std::uint64_t component_id) const {
    auto it = component_interfaces_.find(component_id);
    if (it != component_interfaces_.end()) return it->second;
    return {};
}

bool Assembly::validateMates() const {
    for (const auto& mate : mates_) {
        const AssemblyComponent* comp_a = findComponent(mate.component_a);
        const AssemblyComponent* comp_b = findComponent(mate.component_b);
        
        if (!comp_a || !comp_b) {
            return false;
        }
        
        if (comp_a->id == comp_b->id) {
            return false;
        }
    }
    return true;
}

int Assembly::getDegreesOfFreedom() const {
    int component_count = static_cast<int>(components_.size());
    int total_dof = component_count * 6;
    
    int constraint_dof = 0;
    for (const auto& mate : mates_) {
        switch (mate.type) {
            case MateType::Mate:
                constraint_dof += 3;
                break;
            case MateType::Flush:
            case MateType::Tangent:
                constraint_dof += 1;
                break;
            case MateType::Concentric:
                constraint_dof += 4;
                break;
            case MateType::Angle:
                constraint_dof += 1;
                break;
            case MateType::Insert:
                constraint_dof += 5;
                break;
            case MateType::Gear:
            case MateType::Cam:
                constraint_dof += 1;
                break;
            case MateType::Parallel:
                constraint_dof += 2;
                break;
            case MateType::Distance:
                constraint_dof += 1;
                break;
        }
    }
    
    return total_dof - constraint_dof;
}

bool Assembly::isOverConstrained() const {
    int dof = getDegreesOfFreedom();
    return dof < 0;
}

bool Assembly::isUnderConstrained() const {
    int dof = getDegreesOfFreedom();
    int component_count = static_cast<int>(components_.size());
    return dof > component_count * 3 && component_count > 1;
}

Part Modeler::createPart(const Sketch& sketch) const {
    return Part(sketch.name());
}

Assembly Modeler::createAssembly() const {
    return Assembly();
}

bool Modeler::validateSketch(const Sketch& sketch) const {
    return !sketch.name().empty();
}

namespace {

bool parseNumber(const std::string& text, double& out) {
    char* end = nullptr;
    out = std::strtod(text.c_str(), &end);
    return end != text.c_str();
}

bool evaluateExpression(const std::string& expression,
                        const std::unordered_map<std::string, double>& symbols,
                        double& out) {
    std::string trimmed;
    for (char c : expression) {
        if (!std::isspace(static_cast<unsigned char>(c))) {
            trimmed.push_back(c);
        }
    }

    if (trimmed.empty()) {
        return false;
    }

    size_t plus = trimmed.find('+');
    size_t minus = trimmed.find('-');
    size_t mul = trimmed.find('*');
    size_t div = trimmed.find('/');

    size_t op_pos = std::string::npos;
    char op = '\0';

    if (plus != std::string::npos) {
        op_pos = plus;
        op = '+';
    } else if (minus != std::string::npos) {
        op_pos = minus;
        op = '-';
    } else if (mul != std::string::npos) {
        op_pos = mul;
        op = '*';
    } else if (div != std::string::npos) {
        op_pos = div;
        op = '/';
    }

    if (op_pos == std::string::npos) {
        auto found = symbols.find(trimmed);
        if (found != symbols.end()) {
            out = found->second;
            return true;
        }
        return parseNumber(trimmed, out);
    }

    std::string left = trimmed.substr(0, op_pos);
    std::string right = trimmed.substr(op_pos + 1);

    double lhs = 0.0;
    double rhs = 0.0;
    auto left_symbol = symbols.find(left);
    auto right_symbol = symbols.find(right);
    if (left_symbol != symbols.end()) {
        lhs = left_symbol->second;
    } else if (!parseNumber(left, lhs)) {
        return false;
    }
    if (right_symbol != symbols.end()) {
        rhs = right_symbol->second;
    } else if (!parseNumber(right, rhs)) {
        return false;
    }

    switch (op) {
        case '+':
            out = lhs + rhs;
            return true;
        case '-':
            out = lhs - rhs;
            return true;
        case '*':
            out = lhs * rhs;
            return true;
        case '/':
            if (rhs == 0.0) {
                return false;
            }
            out = lhs / rhs;
            return true;
        default:
            return false;
    }
}

bool evaluateCondition(const std::string& expression,
                       const std::unordered_map<std::string, double>& symbols,
                       bool& out) {
    std::string trimmed;
    for (char c : expression) {
        if (!std::isspace(static_cast<unsigned char>(c))) {
            trimmed.push_back(c);
        }
    }
    if (trimmed.empty()) {
        return false;
    }
    const char* ops2[] = { ">=", "<=", "==", "!=" };
    const char* ops1[] = { ">", "<" };
    size_t op_pos = std::string::npos;
    int op_len = 0;
    int op_type = -1;  // 0: >=, 1: <=, 2: ==, 3: !=, 4: >, 5: <
    for (int i = 0; i < 4; ++i) {
        size_t p = trimmed.find(ops2[i]);
        if (p != std::string::npos && (op_pos == std::string::npos || p < op_pos)) {
            op_pos = p;
            op_len = 2;
            op_type = i;
        }
    }
    for (int i = 0; i < 2; ++i) {
        size_t p = trimmed.find(ops1[i]);
        if (p != std::string::npos && (op_pos == std::string::npos || p < op_pos)) {
            op_pos = p;
            op_len = 1;
            op_type = 4 + i;
        }
    }
    if (op_pos == std::string::npos) {
        return false;
    }
    std::string left = trimmed.substr(0, op_pos);
    std::string right = trimmed.substr(op_pos + op_len);
    double lhs = 0.0, rhs = 0.0;
    if (!evaluateExpression(left, symbols, lhs) || !evaluateExpression(right, symbols, rhs)) {
        return false;
    }
    switch (op_type) {
        case 0: out = (lhs >= rhs); return true;
        case 1: out = (lhs <= rhs); return true;
        case 2: out = (std::abs(lhs - rhs) < 1e-12); return true;
        case 3: out = (std::abs(lhs - rhs) >= 1e-12); return true;
        case 4: out = (lhs > rhs); return true;
        case 5: out = (lhs < rhs); return true;
        default: return false;
    }
}

}  // namespace

bool Modeler::evaluateParameters(Sketch& sketch) const {
    bool all_ok = true;
    std::unordered_map<std::string, double> symbols;
    for (const auto& parameter : sketch.parameters()) {
        symbols[parameter.name] = parameter.value;
    }

    for (auto& parameter : sketch.parameters()) {
        if (parameter.expression.empty()) {
            continue;
        }
        double value = 0.0;
        if (evaluateExpression(parameter.expression, symbols, value)) {
            parameter.value = value;
            symbols[parameter.name] = value;
        } else {
            all_ok = false;
        }
    }
    return all_ok;
}

bool Modeler::evaluatePartParameters(Part& part) const {
    bool all_ok = true;
    std::unordered_map<std::string, double> symbols;
    for (const auto& p : part.userParameters()) {
        symbols[p.name] = p.value;
    }
    for (auto& p : part.userParameters()) {
        if (p.expression.empty()) {
            continue;
        }
        double value = 0.0;
        if (evaluateExpression(p.expression, symbols, value)) {
            p.value = value;
            symbols[p.name] = value;
        } else {
            all_ok = false;
        }
    }
    return all_ok;
}

bool Modeler::evaluatePartRules(Part& part) const {
    std::unordered_map<std::string, double> symbols;
    for (const auto& p : part.userParameters()) {
        symbols[p.name] = p.value;
    }
    bool any_applied = false;
    for (const auto& rule : part.rules()) {
        if (rule.condition_expression.empty() || rule.then_parameter.empty()) {
            continue;
        }
        bool cond = false;
        if (!evaluateCondition(rule.condition_expression, symbols, cond)) {
            continue;
        }
        if (!cond) {
            continue;
        }
        double value = 0.0;
        if (!evaluateExpression(rule.then_value_expression.empty() ? "0" : rule.then_value_expression, symbols, value)) {
            continue;
        }
        part.setParameterValue(rule.then_parameter, value);
        symbols[rule.then_parameter] = value;
        any_applied = true;
    }
    if (any_applied) {
        evaluatePartParameters(part);
    }
    return true;
}

bool Modeler::solveConstraints(Sketch& sketch) const {
    const std::vector<Constraint>& constraints = sketch.constraints();
    std::vector<GeometryEntity>& geometry = const_cast<std::vector<GeometryEntity>&>(sketch.geometry());
    
    if (constraints.empty() || geometry.empty()) {
        return true;
    }
    
    std::set<std::string> fixed_ids;
    for (const auto& c : constraints) {
        if (c.type == ConstraintType::Fixed && !c.a.empty()) {
            fixed_ids.insert(c.a);
        }
    }
    
    int max_iterations = 200;
    double tolerance = 1e-8;
    double damping_factor = 0.5;
    double convergence_rate = 0.1;
    
    std::vector<double> residuals(constraints.size(), 0.0);
    std::vector<std::vector<double>> jacobian(constraints.size());
    for (auto& row : jacobian) {
        row.resize(geometry.size() * 4, 0.0);
    }
    
    std::map<std::string, int> geometry_index_map;
    for (size_t i = 0; i < geometry.size(); ++i) {
        geometry_index_map[geometry[i].id] = static_cast<int>(i);
    }
    
    for (int iter = 0; iter < max_iterations; ++iter) {
        double max_residual = 0.0;
        
        for (size_t c_idx = 0; c_idx < constraints.size(); ++c_idx) {
            const auto& constraint = constraints[c_idx];
            GeometryEntity* geom_a = sketch.findGeometry(constraint.a);
            GeometryEntity* geom_b = sketch.findGeometry(constraint.b);
            
            if (!geom_a && !geom_b) {
                residuals[c_idx] = 0.0;
                continue;
            }
            
            double residual = 0.0;
            double weight = 1.0;
            
            switch (constraint.type) {
                case ConstraintType::Distance:
                    if (geom_a && geom_b) {
                        double dx = geom_b->start_point.x - geom_a->start_point.x;
                        double dy = geom_b->start_point.y - geom_a->start_point.y;
                        double current_dist = std::sqrt(dx*dx + dy*dy);
                        residual = current_dist - constraint.value;
                        weight = 1.0 / (1.0 + current_dist * current_dist);
                    }
                    break;
                case ConstraintType::Horizontal:
                    if (geom_a && geom_b) {
                        residual = geom_b->start_point.y - geom_a->start_point.y;
                    }
                    break;
                case ConstraintType::Vertical:
                    if (geom_a && geom_b) {
                        residual = geom_b->start_point.x - geom_a->start_point.x;
                    }
                    break;
                case ConstraintType::Coincident:
                    if (geom_a && geom_b) {
                        double dx = geom_b->start_point.x - geom_a->start_point.x;
                        double dy = geom_b->start_point.y - geom_a->start_point.y;
                        residual = std::sqrt(dx*dx + dy*dy);
                    }
                    break;
                case ConstraintType::Parallel:
                    if (geom_a && geom_b) {
                        double dx_a = geom_a->end_point.x - geom_a->start_point.x;
                        double dy_a = geom_a->end_point.y - geom_a->start_point.y;
                        double dx_b = geom_b->end_point.x - geom_b->start_point.x;
                        double dy_b = geom_b->end_point.y - geom_b->start_point.y;
                        double cross = dx_a * dy_b - dy_a * dx_b;
                        residual = cross;
                    }
                    break;
                case ConstraintType::Perpendicular:
                    if (geom_a && geom_b) {
                        double dx_a = geom_a->end_point.x - geom_a->start_point.x;
                        double dy_a = geom_a->end_point.y - geom_a->start_point.y;
                        double dx_b = geom_b->end_point.x - geom_b->start_point.x;
                        double dy_b = geom_b->end_point.y - geom_b->start_point.y;
                        double dot = dx_a * dx_b + dy_a * dy_b;
                        residual = dot;
                    }
                    break;
                case ConstraintType::Tangent:
                    if (geom_a && geom_b) {
                        if (geom_a->type == GeometryType::Circle && geom_b->type == GeometryType::Line) {
                            double dx = geom_b->end_point.x - geom_b->start_point.x;
                            double dy = geom_b->end_point.y - geom_b->start_point.y;
                            double dist_to_center = std::abs(
                                (dy * geom_a->center_point.x - dx * geom_a->center_point.y +
                                 dx * geom_b->start_point.y - dy * geom_b->start_point.x) /
                                std::sqrt(dx*dx + dy*dy)
                            );
                            residual = dist_to_center - geom_a->radius;
                        }
                    }
                    break;
                case ConstraintType::Equal:
                    if (geom_a && geom_b) {
                        if (geom_a->type == GeometryType::Circle && geom_b->type == GeometryType::Circle) {
                            residual = geom_a->radius - geom_b->radius;
                        } else if (geom_a->type == GeometryType::Line && geom_b->type == GeometryType::Line) {
                            double len_a = std::sqrt(
                                (geom_a->end_point.x - geom_a->start_point.x) * (geom_a->end_point.x - geom_a->start_point.x) +
                                (geom_a->end_point.y - geom_a->start_point.y) * (geom_a->end_point.y - geom_a->start_point.y)
                            );
                            double len_b = std::sqrt(
                                (geom_b->end_point.x - geom_b->start_point.x) * (geom_b->end_point.x - geom_b->start_point.x) +
                                (geom_b->end_point.y - geom_b->start_point.y) * (geom_b->end_point.y - geom_b->start_point.y)
                            );
                            residual = len_a - len_b;
                        }
                    }
                    break;
                case ConstraintType::Angle:
                    if (geom_a && geom_b) {
                        double dx_a = geom_a->end_point.x - geom_a->start_point.x;
                        double dy_a = geom_a->end_point.y - geom_a->start_point.y;
                        double dx_b = geom_b->end_point.x - geom_b->start_point.x;
                        double dy_b = geom_b->end_point.y - geom_b->start_point.y;
                        double angle_a = std::atan2(dy_a, dx_a);
                        double angle_b = std::atan2(dy_b, dx_b);
                        double angle_diff = angle_b - angle_a;
                        while (angle_diff > M_PI) angle_diff -= 2.0 * M_PI;
                        while (angle_diff < -M_PI) angle_diff += 2.0 * M_PI;
                        double target_angle = constraint.value * M_PI / 180.0;
                        residual = angle_diff - target_angle;
                    }
                    break;
                case ConstraintType::Fixed:
                    residual = 0.0;
                    break;
                case ConstraintType::Symmetric:
                    if (geom_a && geom_b) {
                        double axis_angle = constraint.value * M_PI / 180.0;
                        double cx = std::cos(axis_angle);
                        double cy = std::sin(axis_angle);
                        double d_a = geom_a->start_point.x * cx + geom_a->start_point.y * cy;
                        double d_b = geom_b->start_point.x * cx + geom_b->start_point.y * cy;
                        residual = d_a + d_b;
                    }
                    break;
            }
            
            residuals[c_idx] = residual * weight;
            max_residual = std::max(max_residual, std::abs(residuals[c_idx]));
            
            if (geom_a) {
                auto it_a = geometry_index_map.find(constraint.a);
                if (it_a != geometry_index_map.end()) {
                    int idx = it_a->second * 4;
                    jacobian[c_idx][idx] = -weight * convergence_rate;
                    jacobian[c_idx][idx + 1] = -weight * convergence_rate;
                }
            }
            if (geom_b) {
                auto it_b = geometry_index_map.find(constraint.b);
                if (it_b != geometry_index_map.end()) {
                    int idx = it_b->second * 4;
                    jacobian[c_idx][idx] = weight * convergence_rate;
                    jacobian[c_idx][idx + 1] = weight * convergence_rate;
                }
            }
        }
        
        if (max_residual < tolerance) {
            return true;
        }
        
        for (size_t g_idx = 0; g_idx < geometry.size(); ++g_idx) {
            if (fixed_ids.count(geometry[g_idx].id)) {
                continue;
            }
            double delta_x = 0.0;
            double delta_y = 0.0;
            
            for (size_t c_idx = 0; c_idx < constraints.size(); ++c_idx) {
                int jac_idx = static_cast<int>(g_idx * 4);
                delta_x += jacobian[c_idx][jac_idx] * residuals[c_idx] * damping_factor;
                delta_y += jacobian[c_idx][jac_idx + 1] * residuals[c_idx] * damping_factor;
            }
            
            geometry[g_idx].start_point.x += delta_x;
            geometry[g_idx].start_point.y += delta_y;
            
            if (geometry[g_idx].type == GeometryType::Line || geometry[g_idx].type == GeometryType::Rectangle) {
                geometry[g_idx].end_point.x += delta_x * 0.5;
                geometry[g_idx].end_point.y += delta_y * 0.5;
            }
            
            if (geometry[g_idx].type == GeometryType::Circle || geometry[g_idx].type == GeometryType::Arc ||
                geometry[g_idx].type == GeometryType::Ellipse) {
                geometry[g_idx].center_point.x += delta_x;
                geometry[g_idx].center_point.y += delta_y;
            }
        }
        
        if (iter > 50 && max_residual > 1.0) {
            damping_factor *= 0.95;
        }
    }
    
    return false;
}

bool Modeler::validateConstraints(const Sketch& sketch) const {
    // Validate that constraints reference valid geometry
    for (const auto& constraint : sketch.constraints()) {
        if (!constraint.a.empty() && !sketch.findGeometry(constraint.a)) {
            return false;  // Constraint references non-existent geometry
        }
        if (!constraint.b.empty() && !sketch.findGeometry(constraint.b)) {
            return false;
        }
    }
    return true;
}

bool Modeler::isOverConstrained(const Sketch& sketch) const {
    std::size_t geometry_count = sketch.geometry().size();
    std::size_t constraint_count = sketch.constraints().size();
    
    int total_dof = 0;
    for (const auto& geom : sketch.geometry()) {
        switch (geom.type) {
            case GeometryType::Point:
                total_dof += 2;
                break;
            case GeometryType::Line:
                total_dof += 4;
                break;
            case GeometryType::Circle:
                total_dof += 3;
                break;
            case GeometryType::Arc:
                total_dof += 5;
                break;
            case GeometryType::Rectangle:
                total_dof += 4;
                break;
            case GeometryType::Ellipse:
            case GeometryType::Polygon:
            case GeometryType::Spline:
                total_dof += 4;
                break;
            case GeometryType::Text:
                total_dof += 2;
                break;
        }
    }
    
    int constraint_dof = 0;
    for (const auto& constraint : sketch.constraints()) {
        switch (constraint.type) {
            case ConstraintType::Coincident:
                constraint_dof += 2;
                break;
            case ConstraintType::Horizontal:
            case ConstraintType::Vertical:
                constraint_dof += 1;
                break;
            case ConstraintType::Distance:
            case ConstraintType::Angle:
                constraint_dof += 1;
                break;
            case ConstraintType::Parallel:
            case ConstraintType::Perpendicular:
            case ConstraintType::Tangent:
            case ConstraintType::Equal:
                constraint_dof += 1;
                break;
            case ConstraintType::Symmetric:
                constraint_dof += 2;
                break;
            case ConstraintType::Fixed:
                constraint_dof += 2;
                break;
        }
    }
    
    return constraint_dof > total_dof;
}

bool Modeler::isUnderConstrained(const Sketch& sketch) const {
    // Simple heuristic: if constraints < geometry_count, likely under-constrained
    std::size_t geometry_count = sketch.geometry().size();
    std::size_t constraint_count = sketch.constraints().size();
    
    // Very simplified check
    return constraint_count < geometry_count && geometry_count > 0;
}

int Modeler::getDegreesOfFreedom(const Sketch& sketch) const {
    // Simplified DOF calculation
    // Each geometry entity contributes DOF based on type
    int total_dof = 0;
    for (const auto& geom : sketch.geometry()) {
        switch (geom.type) {
            case GeometryType::Point:
                total_dof += 2;  // x, y
                break;
            case GeometryType::Line:
                total_dof += 4;  // start x, y, end x, y
                break;
            case GeometryType::Circle:
                total_dof += 3;  // center x, y, radius
                break;
            case GeometryType::Arc:
                total_dof += 5;  // center x, y, radius, start_angle, end_angle
                break;
            case GeometryType::Rectangle:
                total_dof += 4;  // corner x, y, width, height
                break;
            case GeometryType::Ellipse:
            case GeometryType::Polygon:
            case GeometryType::Spline:
                total_dof += 4;
                break;
            case GeometryType::Text:
                total_dof += 2;  // position
                break;
        }
    }
    
    // Each constraint removes DOF (simplified: assume 1 per constraint)
    int removed_dof = static_cast<int>(sketch.constraints().size());
    
    return std::max(0, total_dof - removed_dof);
}

Part Modeler::applyExtrude(Part& part, const std::string& sketch_id, double depth, bool symmetric, ExtrudeMode mode) const {
    std::string feature_name = part.createExtrude(sketch_id, depth, symmetric, mode);
    (void)feature_name;  // Feature created and added to part
    return part;
}

Part Modeler::applyRevolve(Part& part, const std::string& sketch_id, double angle, const std::string& axis) const {
    std::string feature_name = part.createRevolve(sketch_id, angle, axis);
    (void)feature_name;
    return part;
}

Part Modeler::applyLoft(Part& part, const std::vector<std::string>& sketch_ids) const {
    std::string feature_name = part.createLoft(sketch_ids);
    (void)feature_name;
    return part;
}

Part Modeler::applyHole(Part& part, double diameter, double depth, bool through_all) const {
    std::string feature_name = part.createHole(diameter, depth, through_all);
    (void)feature_name;
    return part;
}

Part Modeler::applyFillet(Part& part, double radius, const std::vector<std::string>& edge_ids) const {
    std::string feature_name = part.createFillet(radius, edge_ids);
    (void)feature_name;
    return part;
}

Part Modeler::applySweep(Part& part, const std::string& profile_sketch_id, const std::string& path_sketch_id,
                        double twist_angle, double scale_factor) const {
    std::string feature_name = part.createSweep(profile_sketch_id, path_sketch_id, twist_angle, scale_factor);
    (void)feature_name;
    return part;
}

Part Modeler::applyHelix(Part& part, double radius, double pitch, double revolutions, bool clockwise) const {
    std::string feature_name = part.createHelix(radius, pitch, revolutions, clockwise);
    (void)feature_name;
    return part;
}

Part Modeler::applyChamfer(Part& part, double distance1, double distance2, double angle,
                          const std::vector<std::string>& edge_ids) const {
    std::string feature_name = part.createChamfer(distance1, distance2, angle, edge_ids);
    (void)feature_name;
    return part;
}

Part Modeler::applyShell(Part& part, double wall_thickness, const std::vector<std::string>& face_ids) const {
    std::string feature_name = part.createShell(wall_thickness, face_ids);
    (void)feature_name;
    return part;
}

Part Modeler::applyDraft(Part& part, double angle, const std::string& draft_plane,
                        const std::vector<std::string>& face_ids) const {
    std::string feature_name = part.createDraft(angle, draft_plane, face_ids);
    (void)feature_name;
    return part;
}

Part Modeler::applyMirror(Part& part, const std::string& base_feature, const std::string& mirror_plane,
                         bool merge_result) const {
    std::string feature_name = part.createMirror(base_feature, mirror_plane, merge_result);
    (void)feature_name;
    return part;
}

Part Modeler::applyCircularPattern(Part& part, const std::string& base_feature, int count, double angle_deg,
                                     const std::string& axis) const {
    std::string feature_name = part.createCircularPattern(base_feature, count, angle_deg, axis);
    (void)feature_name;
    return part;
}

Part Modeler::applyPathPattern(Part& part, const std::string& base_feature, const std::string& path_sketch_id,
                               int count, bool equal_spacing) const {
    std::string feature_name = part.createPathPattern(base_feature, path_sketch_id, count, equal_spacing);
    (void)feature_name;
    return part;
}

Part Modeler::applyThinExtrude(Part& part, const std::string& sketch_id, double depth, double wall_thickness,
                              bool symmetric, ExtrudeMode mode) const {
    std::string feature_name = part.createThinExtrude(sketch_id, depth, wall_thickness, symmetric, mode);
    (void)feature_name;
    return part;
}

Part Modeler::applyThread(Part& part, const std::string& thread_standard, double pitch, bool internal) const {
    std::string feature_name = part.createThread(thread_standard, pitch, internal);
    (void)feature_name;
    return part;
}

Part Modeler::applyRib(Part& part, double thickness, const std::string& rib_plane, const std::string& sketch_id) const {
    std::string feature_name = part.createRib(thickness, rib_plane, sketch_id);
    (void)feature_name;
    return part;
}

}  // namespace core
}  // namespace cad
