#pragma once

#include <QString>
#include <QByteArray>
#include <QSettings>
#include <QMainWindow>

namespace cad {
namespace ui {

class DockLayoutManager {
public:
    DockLayoutManager();
    ~DockLayoutManager() = default;
    
    // Save and restore layouts
    void saveLayout(QMainWindow* window, const QString& layoutName);
    bool restoreLayout(QMainWindow* window, const QString& layoutName);
    
    // Default layouts
    void saveDefaultLayout(QMainWindow* window);
    bool restoreDefaultLayout(QMainWindow* window);
    
    // Custom layouts
    QStringList getSavedLayouts() const;
    void deleteLayout(const QString& layoutName);
    void renameLayout(const QString& oldName, const QString& newName);
    
    // Settings persistence
    void saveToSettings(QSettings& settings) const;
    void loadFromSettings(QSettings& settings);
    
    // Layout templates (Inventor, SolidWorks, CATIA styles)
    void applyInventorLayout(QMainWindow* window);
    void applySolidWorksLayout(QMainWindow* window);
    void applyCATIALayout(QMainWindow* window);
    QStringList getAvailableTemplates() const;
    
private:
    QMap<QString, QByteArray> savedLayouts_;
    QByteArray defaultLayout_;
    
    QString getLayoutKey(const QString& layoutName) const;
};

}  // namespace ui
}  // namespace cad
