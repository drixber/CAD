#include "UpdateChecker.h"

#include <array>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <string>

#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
#endif

namespace cad {
namespace core {
namespace updates {
namespace {

struct Semver {
    int major{0};
    int minor{0};
    int patch{0};
};

bool parseSemverTag(const std::string& tag, Semver& out) {
    if (tag.empty()) {
        return false;
    }
    std::string trimmed = tag;
    if (trimmed[0] == 'v' || trimmed[0] == 'V') {
        trimmed = trimmed.substr(1);
    }
    std::stringstream ss(trimmed);
    std::string part;
    if (!std::getline(ss, part, '.')) return false;
    out.major = std::atoi(part.c_str());
    if (!std::getline(ss, part, '.')) return false;
    out.minor = std::atoi(part.c_str());
    if (!std::getline(ss, part, '.')) return false;
    out.patch = std::atoi(part.c_str());
    return true;
}

bool isNewer(const Semver& latest, const Semver& current) {
    if (latest.major != current.major) return latest.major > current.major;
    if (latest.minor != current.minor) return latest.minor > current.minor;
    return latest.patch > current.patch;
}

bool runCommandCapture(const std::string& command, std::string& output) {
#ifdef _WIN32
    FILE* pipe = _popen(command.c_str(), "r");
#else
    FILE* pipe = popen(command.c_str(), "r");
#endif
    if (!pipe) {
        return false;
    }
    std::array<char, 256> buffer{};
    while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe)) {
        output.append(buffer.data());
    }
#ifdef _WIN32
    int rc = _pclose(pipe);
#else
    int rc = pclose(pipe);
#endif
    return rc == 0;
}

bool extractJsonStringValue(const std::string& json,
                            const std::string& key,
                            std::string& value) {
    const std::string needle = "\"" + key + "\"";
    size_t pos = json.find(needle);
    if (pos == std::string::npos) {
        return false;
    }
    pos = json.find(':', pos + needle.size());
    if (pos == std::string::npos) {
        return false;
    }
    pos = json.find('"', pos);
    if (pos == std::string::npos) {
        return false;
    }
    size_t end = pos + 1;
    while (end < json.size()) {
        end = json.find('"', end);
        if (end == std::string::npos) {
            return false;
        }
        if (json[end - 1] != '\\') {
            break;
        }
        ++end;
    }
    if (end <= pos + 1) {
        return false;
    }
    value = json.substr(pos + 1, end - pos - 1);
    return true;
}

/** Find first asset with name matching preferred or fallback, return its browser_download_url. */
std::string extractAssetDownloadUrl(const std::string& json,
                                    const std::string& preferred_name,
                                    const std::string& fallback_name) {
    auto try_asset = [&json](const std::string& name) -> std::string {
        const std::string name_key = "\"name\":\"" + name + "\"";
        size_t name_pos = json.find(name_key);
        if (name_pos == std::string::npos) return {};
        const std::string url_key = "\"browser_download_url\":\"";
        size_t url_start = json.find(url_key, name_pos);
        if (url_start == std::string::npos) return {};
        url_start += url_key.size();
        size_t url_end = json.find('"', url_start);
        if (url_end == std::string::npos || url_end <= url_start) return {};
        return json.substr(url_start, url_end - url_start);
    };
    std::string url = try_asset(preferred_name);
    if (!url.empty()) return url;
    return try_asset(fallback_name);
}

}  // namespace

UpdateInfo parseGithubReleaseResponse(const std::string& response, const std::string& currentTag) {
    UpdateInfo info{};
    if (response.empty()) {
        info.error = "Empty response from GitHub API";
        return info;
    }
    std::string latest_tag;
    std::string html_url;
    if (!extractJsonStringValue(response, "tag_name", latest_tag)) {
        info.error = "Failed to parse tag_name";
        return info;
    }
    if (!extractJsonStringValue(response, "html_url", html_url)) {
        info.error = "Failed to parse html_url";
        return info;
    }
    info.latestTag = latest_tag;
    info.releaseUrl = html_url;
    info.assetDownloadUrl = extractAssetDownloadUrl(response, "HydraCADSetup.exe", "app-windows.zip");
    Semver latest{};
    Semver current{};
    if (!parseSemverTag(latest_tag, latest) || !parseSemverTag(currentTag, current)) {
        info.error = "Invalid version format";
        return info;
    }
    info.updateAvailable = isNewer(latest, current);
    return info;
}

UpdateInfo checkGithubLatestRelease(const std::string& owner,
                                    const std::string& repo,
                                    const std::string& currentTag) {
    UpdateInfo info{};
    if (owner.empty() || repo.empty()) {
        info.error = "Owner/Repo not set";
        return info;
    }

    const std::string url = "https://api.github.com/repos/" + owner + "/" + repo + "/releases/latest";
    const std::string command = "curl -s -L -H \"User-Agent: CAD-UpdateChecker\" \"" + url + "\"";
    std::string response;
    if (!runCommandCapture(command, response)) {
        info.error = "curl failed or is not available";
        return info;
    }
    return parseGithubReleaseResponse(response, currentTag);
}

bool openUrlInBrowser(const std::string& url) {
#ifdef _WIN32
    if (url.empty()) {
        return false;
    }
    return reinterpret_cast<intptr_t>(ShellExecuteA(nullptr, "open", url.c_str(), nullptr, nullptr, SW_SHOWNORMAL)) > 32;
#elif __APPLE__
    if (url.empty()) {
        return false;
    }
    std::string cmd = "open \"" + url + "\"";
    return std::system(cmd.c_str()) == 0;
#else
    if (url.empty()) {
        return false;
    }
    std::string cmd = "xdg-open \"" + url + "\"";
    return std::system(cmd.c_str()) == 0;
#endif
}

}  // namespace updates
}  // namespace core
}  // namespace cad
