#pragma once

#include <QString>
#include <QColor>
#include <QMap>
#include <QSettings>

namespace cad {
namespace ui {

struct ColorPalette {
    // Primary colors (Inventor-style)
    QColor primary{0x2B78E4};           // Blue accent
    QColor primaryDark{0x1E5AA8};      // Darker blue
    QColor primaryLight{0x5A9DFF};     // Lighter blue
    
    // Background colors
    QColor background{0xF5F5F5};       // Main background
    QColor surface{0xFFFFFF};          // Panel/surface background
    QColor surfaceHover{0xF0F0F0};     // Hover state
    QColor surfaceSelected{0xE3F2FD};  // Selected state
    
    // Text colors
    QColor textPrimary{0x212121};      // Primary text
    QColor textSecondary{0x757575};    // Secondary text
    QColor textDisabled{0xBDBDBD};     // Disabled text
    QColor textOnPrimary{0xFFFFFF};    // Text on primary color
    
    // Border colors
    QColor border{0xD0D0D0};           // Standard border
    QColor borderLight{0xE8E8E8};      // Light border
    QColor borderDark{0xB0B0B0};       // Dark border
    QColor borderFocus{0x2B78E4};      // Focus border
    
    // Ribbon colors
    QColor ribbonBackground{0xFFFFFF};  // Ribbon background
    QColor ribbonTab{0xF5F5F5};        // Ribbon tab
    QColor ribbonTabSelected{0xFFFFFF}; // Selected tab
    QColor ribbonTabHover{0xEEEEEE};   // Hover tab
    
    // Dock widget colors
    QColor dockTitle{0xF0F0F0};        // Dock title bar
    QColor dockTitleActive{0xE0E0E0};  // Active dock title
    QColor dockContent{0xFFFFFF};      // Dock content
    
    // Status bar colors
    QColor statusBar{0xFAFAFA};        // Status bar background
    QColor statusBarBorder{0xD0D0D0};  // Status bar border
    
    // Button colors
    QColor button{0xFFFFFF};           // Button background
    QColor buttonHover{0xF5F5F5};      // Button hover
    QColor buttonPressed{0xE0E0E0};    // Button pressed
    QColor buttonPrimary{0x2B78E4};    // Primary button
    QColor buttonPrimaryHover{0x1E5AA8}; // Primary button hover
    
    // Input colors
    QColor inputBackground{0xFFFFFF};  // Input background
    QColor inputBorder{0xD0D0D0};      // Input border
    QColor inputBorderFocus{0x2B78E4}; // Input focus border
    
    // Tree/List colors
    QColor treeBackground{0xFFFFFF};    // Tree background
    QColor treeItemHover{0xF5F5F5};    // Tree item hover
    QColor treeItemSelected{0xE3F2FD}; // Tree item selected
    
    // Scrollbar colors
    QColor scrollbar{0xE0E0E0};        // Scrollbar background
    QColor scrollbarHandle{0xC0C0C0};  // Scrollbar handle
    QColor scrollbarHandleHover{0xA0A0A0}; // Scrollbar handle hover
};

class ThemeManager {
public:
    static ThemeManager& instance();
    
    // Theme management
    void setTheme(const QString& themeName);
    QString getCurrentTheme() const;
    QStringList getAvailableThemes() const;
    
    // Color palette
    const ColorPalette& getPalette() const { return palette_; }
    QColor getColor(const QString& colorName) const;
    
    // Stylesheet generation
    QString generateStylesheet() const;
    void applyStylesheet(QWidget* widget) const;
    
    // Settings persistence
    void saveSettings(QSettings& settings) const;
    void loadSettings(QSettings& settings);
    
    // Customization
    void setCustomColor(const QString& colorName, const QColor& color);
    void resetToDefault();
    
private:
    ThemeManager();
    ~ThemeManager() = default;
    ThemeManager(const ThemeManager&) = delete;
    ThemeManager& operator=(const ThemeManager&) = delete;
    
    void initializeThemes();
    void updatePalette(const QString& themeName);
    
    ColorPalette palette_;
    QString currentTheme_;
    QMap<QString, ColorPalette> themes_;
    QMap<QString, QColor> customColors_;
};

}  // namespace ui
}  // namespace cad
