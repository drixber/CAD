#pragma once

#include <string>
#include <vector>
#include <map>

namespace cad {
namespace interop {

struct StepEntity {
    int id{0};
    std::string type;
    std::map<std::string, std::string> parameters;
};

class StepFileParser {
public:
    bool parseFile(const std::string& path);
    std::vector<StepEntity> getEntities() const { return entities_; }
    bool hasHeader() const { return has_header_; }
    bool hasData() const { return has_data_; }
    
private:
    std::vector<StepEntity> entities_;
    bool has_header_{false};
    bool has_data_{false};
    
    StepEntity parseEntity(const std::string& line);
    std::string extractEntityType(const std::string& line);
    std::map<std::string, std::string> extractParameters(const std::string& line);
};

}  // namespace interop
}  // namespace cad

