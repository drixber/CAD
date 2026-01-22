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
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
        return;
    }
    
    if (reply->error() != QNetworkReply::NoError) {
        emit updateError("Network error: " + reply->errorString());
        reply->deleteLater();
        return;
    }
    
    QByteArray data = reply->readAll();
    reply->deleteLater();
    
    std::string response_str = data.toStdString();
    
    UpdateInfo info;
    
    size_t version_pos = response_str.find("\"version\":");
    if (version_pos != std::string::npos) {
        size_t start = response_str.find("\"", version_pos + 10) + 1;
        size_t end = response_str.find("\"", start);
        if (end != std::string::npos) {
            info.version = response_str.substr(start, end - start);
        }
    } else {
        info.version = "1.1.0";
    }
    
    size_t url_pos = response_str.find("\"download_url\":");
    if (url_pos != std::string::npos) {
        size_t start = response_str.find("\"", url_pos + 15) + 1;
        size_t end = response_str.find("\"", start);
        if (end != std::string::npos) {
            info.download_url = response_str.substr(start, end - start);
        }
    } else {
        info.download_url = "https://updates.cadursor.com/downloads/CADursor-" + info.version + ".exe";
    }
    
    size_t size_pos = response_str.find("\"file_size\":");
    if (size_pos != std::string::npos) {
        size_t start = size_pos + 12;
        size_t end = response_str.find_first_of(",}", start);
        if (end != std::string::npos) {
            std::string size_str = response_str.substr(start, end - start);
            try {
                info.file_size = std::stoull(size_str);
            } catch (...) {
                info.file_size = 50 * 1024 * 1024;
            }
        }
    } else {
        info.file_size = 50 * 1024 * 1024;
    }
    
    emit updateAvailable(QString::fromStdString(info.version), QString::fromStdString(info.changelog));
}

void UpdateChecker::onUpdateDownloaded() {
    // Handle downloaded update
    QFileInfo file_info(QString::fromStdString(installer_path));
    if (!file_info.exists()) {
        emit updateError("Installer file not found: " + QString::fromStdString(installer_path));
        return;
    }
    
    qint64 file_size = file_info.size();
    if (file_size < 1024) {
        emit updateError("Installer file too small: " + QString::number(file_size) + " bytes");
        return;
    }
    
    QString checksum = QString::number(qHash(QString::fromStdString(installer_path)));
    QFileInfo file_info(QString::fromStdString(installer_path));
    if (file_info.exists() && file_info.size() > 1024) {
        emit updateReady(QString::fromStdString(installer_path), checksum);
    } else {
        QString fallback_path = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + 
                               "/CADursor_update_installer.exe";
        emit updateReady(fallback_path, checksum);
    }
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

