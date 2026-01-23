#include "Modeler.h"

#include <utility>
#include <cctype>
#include <cstdlib>
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

std::string Part::createExtrude(const std::string& sketch_id, double depth, bool symmetric) {
    Feature feature;
    feature.name = generateFeatureName(FeatureType::Extrude);
    feature.type = FeatureType::Extrude;
    feature.sketch_id = sketch_id;
    feature.depth = depth;
    feature.symmetric = symmetric;
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
                constraint_dof += 1;
                break;
            case MateType::Angle:
                constraint_dof += 1;
                break;
            case MateType::Insert:
                constraint_dof += 5;
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

bool Modeler::solveConstraints(Sketch& sketch) const {
    const std::vector<Constraint>& constraints = sketch.constraints();
    std::vector<GeometryEntity>& geometry = const_cast<std::vector<GeometryEntity>&>(sketch.geometry());
    
    if (constraints.empty() || geometry.empty()) {
        return true;
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
            
            if (geometry[g_idx].type == GeometryType::Circle || geometry[g_idx].type == GeometryType::Arc) {
                geometry[g_idx].center_point.x += delta_x;
                geometry[g_idx].center_point.y += delta_y;
            }
        }
        
        if (iter > 50 && max_residual > 1.0) {
            damping_factor *= 0.95;
        }
    }
    
    return false;
                    geom_b->start_point = geom_a->start_point;
                }
                break;
            case ConstraintType::Parallel:
                if (geom_a && geom_b && geom_a->type == GeometryType::Line && geom_b->type == GeometryType::Line) {
                    // Make lines parallel (same slope)
                    double dx_a = geom_a->end_point.x - geom_a->start_point.x;
                    double dy_a = geom_a->end_point.y - geom_a->start_point.y;
                    double dx_b = geom_b->end_point.x - geom_b->start_point.x;
                    double dy_b = geom_b->end_point.y - geom_b->start_point.y;
                    
                    if (std::abs(dx_a) > 0.001) {
                        double slope = dy_a / dx_a;
                        geom_b->end_point.y = geom_b->start_point.y + slope * dx_b;
                    } else {
                        // Vertical line - make second line vertical too
                        geom_b->end_point.x = geom_b->start_point.x;
                    }
                }
                break;
            case ConstraintType::Perpendicular:
                if (geom_a && geom_b && geom_a->type == GeometryType::Line && geom_b->type == GeometryType::Line) {
                    // Make lines perpendicular (negative reciprocal slope)
                    double dx_a = geom_a->end_point.x - geom_a->start_point.x;
                    double dy_a = geom_a->end_point.y - geom_a->start_point.y;
                    double dx_b = geom_b->end_point.x - geom_b->start_point.x;
                    
                    if (std::abs(dx_a) > 0.001) {
                        double slope_a = dy_a / dx_a;
                        double slope_b = -1.0 / slope_a;
                        geom_b->end_point.y = geom_b->start_point.y + slope_b * dx_b;
                    } else {
                        // First line is vertical, make second horizontal
                        geom_b->end_point.y = geom_b->start_point.y;
                    }
                }
                break;
            case ConstraintType::Tangent:
                if (geom_a && geom_b) {
                    // Tangent constraint between line and circle/arc
                    if (geom_a->type == GeometryType::Line && 
                        (geom_b->type == GeometryType::Circle || geom_b->type == GeometryType::Arc)) {
                        // Calculate distance from circle center to line
                        // Adjust line or circle to make them tangent
                        double dx = geom_a->end_point.x - geom_a->start_point.x;
                        double dy = geom_a->end_point.y - geom_a->start_point.y;
                        double length = std::sqrt(dx*dx + dy*dy);
                        if (length > 0.001) {
                            double dist_to_center = std::abs(
                                (dy * geom_b->center_point.x - dx * geom_b->center_point.y + 
                                 dx * geom_a->start_point.y - dy * geom_a->start_point.x) / length
                            );
                            // Adjust to make distance equal to radius
                            if (std::abs(dist_to_center - geom_b->radius) > 0.001) {
                                // Simple adjustment: move line perpendicular to make tangent
                                double offset = dist_to_center - geom_b->radius;
                                double perp_x = -dy / length;
                                double perp_y = dx / length;
                                geom_a->start_point.x += perp_x * offset;
                                geom_a->start_point.y += perp_y * offset;
                                geom_a->end_point.x += perp_x * offset;
                                geom_a->end_point.y += perp_y * offset;
                            }
                        }
                    }
                }
                break;
            case ConstraintType::Equal:
                if (geom_a && geom_b) {
                    // Make geometries equal (length for lines, radius for circles)
                    if (geom_a->type == GeometryType::Line && geom_b->type == GeometryType::Line) {
                        // Make line lengths equal
                        double dx_a = geom_a->end_point.x - geom_a->start_point.x;
                        double dy_a = geom_a->end_point.y - geom_a->start_point.y;
                        double length_a = std::sqrt(dx_a*dx_a + dy_a*dy_a);
                        
                        double dx_b = geom_b->end_point.x - geom_b->start_point.x;
                        double dy_b = geom_b->end_point.y - geom_b->start_point.y;
                        double length_b = std::sqrt(dx_b*dx_b + dy_b*dy_b);
                        
                        if (length_a > 0.001 && length_b > 0.001) {
                            double scale = length_a / length_b;
                            geom_b->end_point.x = geom_b->start_point.x + dx_b * scale;
                            geom_b->end_point.y = geom_b->start_point.y + dy_b * scale;
                        }
                    } else if (geom_a->type == GeometryType::Circle && geom_b->type == GeometryType::Circle) {
                        // Make radii equal
                        geom_b->radius = geom_a->radius;
                    }
                }
                break;
            case ConstraintType::Angle:
                if (geom_a && geom_b && geom_a->type == GeometryType::Line && geom_b->type == GeometryType::Line) {
                    // Apply angle constraint between two lines
                    double dx_a = geom_a->end_point.x - geom_a->start_point.x;
                    double dy_a = geom_a->end_point.y - geom_a->start_point.y;
                    double angle_a = std::atan2(dy_a, dx_a);
                    double target_angle = angle_a + constraint.value * M_PI / 180.0;
                    
                    double dx_b = geom_b->end_point.x - geom_b->start_point.x;
                    double dy_b = geom_b->end_point.y - geom_b->start_point.y;
                    double length_b = std::sqrt(dx_b*dx_b + dy_b*dy_b);
                    
                    if (length_b > 0.001) {
                        geom_b->end_point.x = geom_b->start_point.x + length_b * std::cos(target_angle);
                        geom_b->end_point.y = geom_b->start_point.y + length_b * std::sin(target_angle);
                    }
                }
                break;
        }
    }
    
    return true;
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
        }
    }
    
    // Each constraint removes DOF (simplified: assume 1 per constraint)
    int removed_dof = static_cast<int>(sketch.constraints().size());
    
    return std::max(0, total_dof - removed_dof);
}

Part Modeler::applyExtrude(Part& part, const std::string& sketch_id, double depth, bool symmetric) const {
    std::string feature_name = part.createExtrude(sketch_id, depth, symmetric);
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
