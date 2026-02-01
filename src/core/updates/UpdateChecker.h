#pragma once

#include <string>
#include <map>

namespace cad {
namespace core {
namespace updates {

struct UpdateInfo {
    bool updateAvailable{false};
    std::string releaseUrl;
    /** Direct download URL for an asset (e.g. HydraCADSetup.exe or app-windows.zip). Empty if none found. */
    std::string assetDownloadUrl;
    std::string latestTag;
    /** Release notes body from GitHub (markdown/text). Empty if not parsed. */
    std::string body;
    std::string error;
};

/** Parses GitHub API "releases/latest" JSON and returns UpdateInfo. Use when response was fetched via HTTP (e.g. Qt). */
UpdateInfo parseGithubReleaseResponse(const std::string& response, const std::string& currentTag);

/** Returns browser_download_url for the given asset name from a GitHub release JSON (e.g. "update.json"). */
std::string getAssetUrlFromReleaseJson(const std::string& releaseJson, const std::string& assetName);

/** Parses update.json asset body; returns map asset_name -> sha256 (hex). Empty if parse fails. */
std::map<std::string, std::string> parseUpdateJsonChecksums(const std::string& updateJsonBody);

/** Compare two semver tags (e.g. v1.2.3 and 1.2.4). Returns true if latest > current. */
bool isVersionNewerThan(const std::string& latestTag, const std::string& currentTag);

/** Fetches via curl (requires curl on system). Prefer fetch in app with Qt + parseGithubReleaseResponse on Windows. */
UpdateInfo checkGithubLatestRelease(const std::string& owner,
                                    const std::string& repo,
                                    const std::string& currentTag);
bool openUrlInBrowser(const std::string& url);

}  // namespace updates
}  // namespace core
}  // namespace cad
