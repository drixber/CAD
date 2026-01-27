#pragma once

#include <string>

namespace cad {
namespace core {
namespace updates {

struct UpdateInfo {
    bool updateAvailable{false};
    std::string releaseUrl;
    std::string latestTag;
    std::string error;
};

UpdateInfo checkGithubLatestRelease(const std::string& owner,
                                    const std::string& repo,
                                    const std::string& currentTag);
bool openUrlInBrowser(const std::string& url);

}  // namespace updates
}  // namespace core
}  // namespace cad
