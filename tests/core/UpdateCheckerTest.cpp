#include <gtest/gtest.h>
#include "core/updates/UpdateChecker.h"

using namespace cad::core::updates;

TEST(UpdateCheckerTest, IsVersionNewerThan_Basic) {
    EXPECT_TRUE(isVersionNewerThan("v2.0.0", "v1.0.0"));
    EXPECT_TRUE(isVersionNewerThan("v1.1.0", "v1.0.0"));
    EXPECT_TRUE(isVersionNewerThan("v1.0.1", "v1.0.0"));
    EXPECT_FALSE(isVersionNewerThan("v1.0.0", "v1.0.0"));
    EXPECT_FALSE(isVersionNewerThan("v1.0.0", "v2.0.0"));
    EXPECT_FALSE(isVersionNewerThan("v1.0.0", "v1.1.0"));
}

TEST(UpdateCheckerTest, IsVersionNewerThan_AcceptsVprefix) {
    EXPECT_TRUE(isVersionNewerThan("v3.0.16", "3.0.14"));
    EXPECT_TRUE(isVersionNewerThan("3.0.16", "v3.0.14"));
}

TEST(UpdateCheckerTest, IsVersionNewerThan_PrereleaseNotNewerThanStable) {
    EXPECT_FALSE(isVersionNewerThan("v3.0.16-beta", "v3.0.16"));
    EXPECT_FALSE(isVersionNewerThan("v3.0.16-rc.1", "v3.0.16"));
    EXPECT_TRUE(isVersionNewerThan("v3.0.17", "v3.0.16"));
}

TEST(UpdateCheckerTest, ParseGithubReleaseResponse_Empty) {
    UpdateInfo info = parseGithubReleaseResponse("", "v1.0.0");
    EXPECT_FALSE(info.error.empty());
}

TEST(UpdateCheckerTest, ParseGithubReleaseResponse_RateLimit) {
    std::string body = "{\"message\": \"API rate limit exceeded\", \"documentation_url\": \"...\"}";
    UpdateInfo info = parseGithubReleaseResponse(body, "v1.0.0");
    EXPECT_FALSE(info.error.empty());
    EXPECT_TRUE(info.error.find("rate limit") != std::string::npos);
}

TEST(UpdateCheckerTest, ParseUpdateJsonChecksums_Empty) {
    auto m = parseUpdateJsonChecksums("");
    EXPECT_TRUE(m.empty());
}

TEST(UpdateCheckerTest, ParseUpdateJsonChecksums_Valid) {
    std::string json = R"({"version":"v3.0.16","assets":[{"name":"HydraCADSetup.exe","sha256":"abc123"},{"name":"app-windows.zip","sha256":"def456"}]})";
    auto m = parseUpdateJsonChecksums(json);
    EXPECT_EQ(m.size(), 2u);
    EXPECT_EQ(m["HydraCADSetup.exe"], "abc123");
    EXPECT_EQ(m["app-windows.zip"], "def456");
}

TEST(UpdateCheckerTest, GetAssetUrlFromReleaseJson) {
    std::string release = R"({"assets":[{"name":"update.json","browser_download_url":"https://github.com/foo/bar/releases/download/v1.0.0/update.json"}]})";
    std::string url = getAssetUrlFromReleaseJson(release, "update.json");
    EXPECT_FALSE(url.empty());
    EXPECT_TRUE(url.find("update.json") != std::string::npos);
}
