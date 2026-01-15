#pragma once

#include <string>
#include <vector>

namespace cad {
namespace core {

struct Feature {
    std::string name;
    std::string type;
};

class Part {
public:
    explicit Part(std::string name);

    const std::string& name() const;
    void addFeature(const Feature& feature);
    const std::vector<Feature>& features() const;

private:
    std::string name_;
    std::vector<Feature> features_;
};

}  // namespace core
}  // namespace cad
