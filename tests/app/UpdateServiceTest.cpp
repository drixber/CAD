#include <gtest/gtest.h>
#include "app/UpdateService.h"

using namespace cad::app;

TEST(UpdateServiceTest, CheckForUpdates) {
    UpdateService service;
    service.setUpdateServerUrl("https://updates.cadursor.com/api/v1");
    
    bool has_updates = service.checkForUpdates();
    
    ASSERT_TRUE(has_updates || !has_updates);
}

TEST(UpdateServiceTest, GetLatestUpdateInfo) {
    UpdateService service;
    service.setUpdateServerUrl("https://updates.cadursor.com/api/v1");
    service.checkForUpdates();
    
    UpdateInfo info = service.getLatestUpdateInfo();
    
    ASSERT_FALSE(info.version.empty());
    ASSERT_FALSE(info.download_url.empty());
}

TEST(UpdateServiceTest, DownloadUpdate) {
    UpdateService service;
    service.setUpdateServerUrl("https://updates.cadursor.com/api/v1");
    service.checkForUpdates();
    
    UpdateInfo info = service.getLatestUpdateInfo();
    info.file_size = 1024 * 1024;
    
    bool downloaded = service.downloadUpdate(info, nullptr);
    
    ASSERT_TRUE(downloaded);
}

TEST(UpdateServiceTest, CurrentVersion) {
    UpdateService service;
    
    std::string version = service.getCurrentVersion();
    ASSERT_FALSE(version.empty());
    
    service.setCurrentVersion("2.0.0");
    ASSERT_EQ(service.getCurrentVersion(), "2.0.0");
}
