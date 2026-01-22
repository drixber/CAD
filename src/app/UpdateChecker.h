#pragma once

#include <QObject>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#include "UpdateService.h"

namespace cad {
namespace ui {

class UpdateChecker : public QObject {
    Q_OBJECT

public:
    explicit UpdateChecker(QObject* parent = nullptr);
    ~UpdateChecker();
    
    void checkForUpdates();
    void enableAutoCheck(bool enabled);
    void setCheckInterval(int days);
    
signals:
    void updateAvailable(const QString& version, const QString& changelog);
    void updateDownloadProgress(int percentage, const QString& status);
    void updateReady(const QString& installer_path);
    void updateError(const QString& error_message);

private slots:
    void onAutoCheckTimer();
    void onUpdateInfoReceived();
    void onUpdateDownloaded();
    void onNetworkError(QNetworkReply::NetworkError error);

private:
    cad::app::UpdateService* update_service_;
    QTimer* auto_check_timer_;
    QNetworkAccessManager* network_manager_;
    bool auto_check_enabled_{true};
    int check_interval_days_{7};
    
    void scheduleAutoCheck();
    void downloadUpdateInstaller(const QString& url);
};

}  // namespace ui
}  // namespace cad

