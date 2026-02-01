#pragma once

#include <string>
#include <vector>

namespace cad {
namespace modules {

/** Datenverwaltung / Vault (§17.9): Versionen, Check-out/Check-in, Freigabe, Revisionen. Stub. */
class VaultService {
public:
    struct FileVersion {
        std::string version_id;
        std::string path;
        int revision{0};
        std::string comment;
    };
    bool checkOut(const std::string& file_path);
    bool checkIn(const std::string& file_path, const std::string& comment);
    std::vector<FileVersion> getVersionHistory(const std::string& file_path) const;
};

}  // namespace modules
}  // namespace cad
