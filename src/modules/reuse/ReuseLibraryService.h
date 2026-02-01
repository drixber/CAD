#pragma once

#include <string>
#include <vector>

namespace cad {
namespace modules {

/** NX Reuse Library: Wiederverwendbare Komponenten/Katalog. Stub. */
class ReuseLibraryService {
public:
    struct ReuseItem {
        std::string id;
        std::string name;
        std::string category;
        std::string path;
    };
    std::vector<ReuseItem> listItems(const std::string& category) const;
    bool addToPart(const std::string& item_id, const std::string& part_id) const;
};

}  // namespace modules
}  // namespace cad
