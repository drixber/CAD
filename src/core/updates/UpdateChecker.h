#pragma once

#include <string>

namespace cad {
namespace core {
namespace updates {

struct UpdateInfo {
    bool updateAvailable{false};
    std::string releaseUrl;
    /** Direct download URL for an asset (e.g. HydraCADSetup.exe or app-windows.zip). Empty if none found. */
    std::string assetDownloadUrl;
    std::string latestTag;
    std::string error;
};

/** Parses GitHub API "releases/latest" JSON and returns UpdateInfo. Use when response was fetched via HTTP (e.g. Qt). */
UpdateInfo parseGithubReleaseResponse(const std::string& response, const std::string& currentTag);

/** Fetches via curl (requires curl on system). Prefer fetch in app with Qt + parseGithubReleaseResponse on Windows. */
UpdateInfo checkGithubLatestRelease(const std::string& owner,
                                    const std::string& repo,
                                    const std::string& currentTag);
bool openUrlInBrowser(const std::string& url);

}  // namespace updates
}  // namespace core
}  // namespace cad
