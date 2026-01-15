#include "Modeler.h"

#include <utility>
#include <cctype>
#include <cstdlib>
#include <unordered_map>

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

}  // namespace core
}  // namespace cad
