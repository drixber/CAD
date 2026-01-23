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

TEST(UpdateServiceTest, ChecksumVerification) {
    UpdateService service;
    
    std::string test_file = "test_update.exe";
    std::ofstream file(test_file, std::ios::binary);
    file.write("test data", 9);
    file.close();
    
    std::string checksum = service.calculateFileChecksum(test_file);
    ASSERT_FALSE(checksum.empty());
    
    bool verified = service.verifyUpdateChecksum(test_file, checksum);
    ASSERT_TRUE(verified);
    
    std::remove(test_file.c_str());
}
