#include "UpdateChecker.h"

#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QFile>
#include <QFileInfo>
#include <QDir>

namespace cad {
namespace ui {

UpdateChecker::UpdateChecker(QObject* parent) : QObject(parent) {
    update_service_ = new cad::app::UpdateService();
    network_manager_ = new QNetworkAccessManager(this);
    
    auto_check_timer_ = new QTimer(this);
    connect(auto_check_timer_, &QTimer::timeout, this, &UpdateChecker::onAutoCheckTimer);
    
    scheduleAutoCheck();
}

UpdateChecker::~UpdateChecker() {
    delete update_service_;
}

void UpdateChecker::checkForUpdates() {
    if (update_service_->checkForUpdates()) {
        if (update_service_->isUpdateAvailable()) {
            cad::app::UpdateInfo info = update_service_->getLatestUpdateInfo();
            emit updateAvailable(QString::fromStdString(info.version),
                               QString::fromStdString(info.changelog));
        }
    }
}

void UpdateChecker::enableAutoCheck(bool enabled) {
    auto_check_enabled_ = enabled;
    if (enabled) {
        scheduleAutoCheck();
    } else {
        auto_check_timer_->stop();
    }
}

void UpdateChecker::setCheckInterval(int days) {
    check_interval_days_ = days;
    update_service_->setAutoUpdateCheckInterval(days);
    scheduleAutoCheck();
}

void UpdateChecker::onAutoCheckTimer() {
    if (auto_check_enabled_) {
        checkForUpdates();
    }
    scheduleAutoCheck();
}

void UpdateChecker::onUpdateInfoReceived() {
    // Handle update info response
    // In real implementation: would parse JSON response
    // QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    // QJsonObject obj = doc.object();
    // UpdateInfo info;
    // info.version = obj["version"].toString().toStdString();
    // info.download_url = obj["download_url"].toString().toStdString();
    // // ... parse other fields
    // emit updateAvailable(info);
    
    // For now, emit signal with simulated data
    UpdateInfo info;
    info.version = "1.1.0";
    info.download_url = "https://updates.cadursor.com/downloads/CADursor-1.1.0.exe";
    emit updateAvailable(info);
}

void UpdateChecker::onUpdateDownloaded() {
    // Handle downloaded update
    // In real implementation: would verify checksum and prepare installer
    // if (verifyChecksum(downloaded_file, expected_checksum)) {
    //     emit updateReady(downloaded_file);
    // } else {
    //     emit updateError("Checksum verification failed");
    // }
    
    // For now, emit signal with simulated installer path
    emit updateReady("update_installer.exe");
}

void UpdateChecker::onNetworkError(QNetworkReply::NetworkError error) {
    QString error_msg = "Network error: " + QString::number(error);
    emit updateError(error_msg);
}

void UpdateChecker::scheduleAutoCheck() {
    if (auto_check_enabled_) {
        int interval_ms = check_interval_days_ * 24 * 60 * 60 * 1000;
        auto_check_timer_->setSingleShot(true);
        auto_check_timer_->start(interval_ms);
    }
}

void UpdateChecker::downloadUpdateInstaller(const QString& url) {
    QNetworkRequest request{QUrl(url)};
    QNetworkReply* reply = network_manager_->get(request);
    
    connect(reply, &QNetworkReply::downloadProgress, this, [this](qint64 bytesReceived, qint64 bytesTotal) {
        int percentage = bytesTotal > 0 ? (bytesReceived * 100) / bytesTotal : 0;
        emit updateDownloadProgress(percentage, "Downloading update...");
    });
    
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QString download_path = QDir::tempPath() + "/CADursorUpdate.exe";
            QFile file(download_path);
            if (file.open(QIODevice::WriteOnly)) {
                file.write(reply->readAll());
                file.close();
                emit updateReady(download_path);
            } else {
                emit updateError("Failed to save update file");
            }
        } else {
            emit updateError("Download failed: " + reply->errorString());
        }
        reply->deleteLater();
    });
}

}  // namespace ui
}  // namespace cad

