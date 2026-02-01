#pragma once

#include <string>
#include <vector>

namespace cad {
namespace modules {

/** Cloud & Zusammenarbeit (§18.16): Live-Sharing, Kommentare, Markups, Team-Projekte, Rechte. Stub. */
class CollaborationService {
public:
    struct Comment {
        std::string id;
        std::string author;
        std::string text;
        double x{0}, y{0}, z{0};
    };
    bool shareProject(const std::string& project_id, const std::string& user_id, const std::string& role);
    bool addComment(const std::string& project_id, const Comment& comment);
    std::vector<Comment> getComments(const std::string& project_id) const;
    bool setUserRole(const std::string& project_id, const std::string& user_id, const std::string& role);
};

}  // namespace modules
}  // namespace cad
