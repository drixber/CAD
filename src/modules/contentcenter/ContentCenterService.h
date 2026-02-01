#pragma once

#include <string>
#include <vector>

namespace cad {
namespace modules {

/** Inventor Content Center: Normteile, Rahmenprofile, Blech. Stub. */
class ContentCenterService {
public:
    struct ContentItem {
        std::string id;
        std::string family;
        std::string part_number;
        std::string path;
    };
    std::vector<ContentItem> search(const std::string& family, const std::string& keyword) const;
    bool placePart(const std::string& item_id, const std::string& assembly_id) const;
};

}  // namespace modules
}  // namespace cad
