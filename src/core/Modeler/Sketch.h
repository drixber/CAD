#pragma once

#include <string>
#include <vector>

namespace cad {
namespace core {

enum class ConstraintType {
    Coincident,
    Horizontal,
    Vertical,
    Parallel,
    Perpendicular,
    Tangent,
    Equal,
    Distance,
    Angle
};

struct Constraint {
    ConstraintType type{ConstraintType::Coincident};
    std::string a;
    std::string b;
    double value{0.0};
};

struct Parameter {
    std::string name;
    double value{0.0};
    std::string expression;
};

class Sketch {
public:
    explicit Sketch(std::string name);

    const std::string& name() const;
    void addConstraint(const Constraint& constraint);
    const std::vector<Constraint>& constraints() const;
    void addParameter(const Parameter& parameter);
    std::vector<Parameter>& parameters();
    const std::vector<Parameter>& parameters() const;

private:
    std::string name_;
    std::vector<Constraint> constraints_;
    std::vector<Parameter> parameters_;
};

}  // namespace core
}  // namespace cad
