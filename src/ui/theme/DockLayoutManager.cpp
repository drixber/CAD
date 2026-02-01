#include "DockLayoutManager.h"
#include <QMainWindow>
#include <QSettings>
#include <QDebug>
#include <QMenu>
#include <QAction>

namespace cad {
namespace ui {

DockLayoutManager::DockLayoutManager() {
}

void DockLayoutManager::saveLayout(QMainWindow* window, const QString& layoutName) {
    if (!window) return;
    
    QByteArray layoutData = window->saveState();
    savedLayouts_[layoutName] = layoutData;
}

bool DockLayoutManager::restoreLayout(QMainWindow* window, const QString& layoutName) {
    if (!window) return false;
    
    if (savedLayouts_.contains(layoutName)) {
        return window->restoreState(savedLayouts_[layoutName]);
    }
    
    return false;
}

void DockLayoutManager::saveDefaultLayout(QMainWindow* window) {
    if (!window) return;
    
    defaultLayout_ = window->saveState();
}

bool DockLayoutManager::restoreDefaultLayout(QMainWindow* window) {
    if (!window || defaultLayout_.isEmpty()) return false;
    
    return window->restoreState(defaultLayout_);
}

QStringList DockLayoutManager::getSavedLayouts() const {
    return savedLayouts_.keys();
}

void DockLayoutManager::deleteLayout(const QString& layoutName) {
    savedLayouts_.remove(layoutName);
}

void DockLayoutManager::renameLayout(const QString& oldName, const QString& newName) {
    if (savedLayouts_.contains(oldName) && !savedLayouts_.contains(newName)) {
        savedLayouts_[newName] = savedLayouts_[oldName];
        savedLayouts_.remove(oldName);
    }
}

void DockLayoutManager::saveToSettings(QSettings& settings) const {
    settings.beginGroup("DockLayouts");
    settings.setValue("default", defaultLayout_);
    
    settings.beginWriteArray("layouts");
    int i = 0;
    for (auto it = savedLayouts_.constBegin(); it != savedLayouts_.constEnd(); ++it) {
        settings.setArrayIndex(i++);
        settings.setValue("name", it.key());
        settings.setValue("data", it.value());
    }
    settings.endArray();
    settings.endGroup();
}

void DockLayoutManager::loadFromSettings(QSettings& settings) {
    settings.beginGroup("DockLayouts");
    defaultLayout_ = settings.value("default").toByteArray();
    
    int size = settings.beginReadArray("layouts");
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        QString name = settings.value("name").toString();
        QByteArray data = settings.value("data").toByteArray();
        if (!name.isEmpty() && !data.isEmpty()) {
            savedLayouts_[name] = data;
        }
    }
    settings.endArray();
    settings.endGroup();
}

QString DockLayoutManager::getLayoutKey(const QString& layoutName) const {
    return QString("layout_%1").arg(layoutName);
}

void DockLayoutManager::applyInventorLayout(QMainWindow* window) {
    if (!window) return;
    if (!restoreDefaultLayout(window)) {
        saveDefaultLayout(window);
    }
}

void DockLayoutManager::applySolidWorksLayout(QMainWindow* window) {
    if (!window) return;
    if (!restoreDefaultLayout(window)) {
        saveDefaultLayout(window);
    }
}

void DockLayoutManager::applyCATIALayout(QMainWindow* window) {
    if (!window) return;
    if (!restoreDefaultLayout(window)) {
        saveDefaultLayout(window);
    }
}

QStringList DockLayoutManager::getAvailableTemplates() const {
    return QStringList() << "Inventor" << "SolidWorks" << "CATIA" << "Default";
}

}  // namespace ui
}  // namespace cad
