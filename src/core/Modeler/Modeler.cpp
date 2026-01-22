#include "Modeler.h"

#include <utility>
#include <cctype>
#include <cstdlib>
#include <unordered_map>
#include <algorithm>
#include <cmath>

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
    // Simple mate solver - updates component transforms based on mates
    // In real implementation: would use a proper constraint solver
    
    for (const auto& mate : mates_) {
        AssemblyComponent* comp_a = findComponent(mate.component_a);
        AssemblyComponent* comp_b = findComponent(mate.component_b);
        
        if (!comp_a || !comp_b) {
            continue;
        }
        
        switch (mate.type) {
            case MateType::Mate:
                // Align components (simple: move component_b to component_a position + offset)
                comp_b->transform.tx = comp_a->transform.tx + mate.value;
                comp_b->transform.ty = comp_a->transform.ty;
                comp_b->transform.tz = comp_a->transform.tz;
                break;
            case MateType::Flush:
                // Make faces flush
                comp_b->transform.tx = comp_a->transform.tx + mate.value;
                break;
            case MateType::Angle:
                // Apply angle constraint (simple: rotate around Z axis)
                comp_b->transform.rz = comp_a->transform.rz + mate.value;
                break;
            case MateType::Insert:
                // Insert constraint (align centers)
                comp_b->transform.tx = comp_a->transform.tx;
                comp_b->transform.ty = comp_a->transform.ty;
                comp_b->transform.tz = comp_a->transform.tz;
                break;
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
    // Simple constraint solver - applies constraints to geometry
    // In real implementation: would use a proper constraint solver (e.g., PlanarGCS)
    
    for (const auto& constraint : sketch.constraints()) {
        GeometryEntity* geom_a = sketch.findGeometry(constraint.a);
        GeometryEntity* geom_b = sketch.findGeometry(constraint.b);
        
        if (!geom_a && !geom_b) {
            // Constraint might reference parameters or other entities
            continue;
        }
        
        switch (constraint.type) {
            case ConstraintType::Distance:
                if (geom_a && geom_b) {
                    // Apply distance constraint
                    double dx = geom_b->start_point.x - geom_a->start_point.x;
                    double dy = geom_b->start_point.y - geom_a->start_point.y;
                    double current_dist = std::sqrt(dx*dx + dy*dy);
                    if (current_dist > 0.001) {
                        double scale = constraint.value / current_dist;
                        geom_b->start_point.x = geom_a->start_point.x + dx * scale;
                        geom_b->start_point.y = geom_a->start_point.y + dy * scale;
                    }
                }
                break;
            case ConstraintType::Horizontal:
                if (geom_a && geom_b) {
                    // Make line horizontal
                    geom_b->start_point.y = geom_a->start_point.y;
                }
                break;
            case ConstraintType::Vertical:
                if (geom_a && geom_b) {
                    // Make line vertical
                    geom_b->start_point.x = geom_a->start_point.x;
                }
                break;
            case ConstraintType::Coincident:
                if (geom_a && geom_b) {
                    // Make points coincident
                    geom_b->start_point = geom_a->start_point;
                }
                break;
            case ConstraintType::Parallel:
            case ConstraintType::Perpendicular:
            case ConstraintType::Tangent:
            case ConstraintType::Equal:
            case ConstraintType::Angle:
                // More complex constraints - placeholder
                break;
        }
    }
    
    return true;
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

}  // namespace core
}  // namespace cad
