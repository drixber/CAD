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

}  // namespace core
}  // namespace cad
