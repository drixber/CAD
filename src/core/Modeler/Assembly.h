#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "Part.h"
#include "Transform.h"

namespace cad {
namespace core {

struct AssemblyComponent {
    std::uint64_t id{0};
    Part part{"Part"};
    Transform transform{};
};

enum class MateType {
    Mate,
    Flush,
    Angle,
    Insert
};

struct MateConstraint {
    std::uint64_t component_a{0};
    std::uint64_t component_b{0};
    MateType type{MateType::Mate};
    double value{0.0};
};

class Assembly {
public:
    std::uint64_t addComponent(const Part& part, const Transform& transform);
    const std::vector<AssemblyComponent>& components() const;
    void addMate(const MateConstraint& mate);
    const std::vector<MateConstraint>& mates() const;

private:
    std::uint64_t next_id_{1};
    std::vector<AssemblyComponent> components_{};
    std::vector<MateConstraint> mates_{};
};

}  // namespace core
}  // namespace cad
