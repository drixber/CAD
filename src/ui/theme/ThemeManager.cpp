#include "ThemeManager.h"
#include <QApplication>
#include <QWidget>
#include <QFile>
#include <QTextStream>

namespace cad {
namespace ui {

ThemeManager& ThemeManager::instance() {
    static ThemeManager instance;
    return instance;
}

ThemeManager::ThemeManager() {
    initializeThemes();
    setTheme("InventorModern");
}

void ThemeManager::initializeThemes() {
    // Inventor Modern Theme
    ColorPalette inventorModern;
    inventorModern.primary = QColor::fromRgb(0x2B78E4);
    inventorModern.primaryDark = QColor::fromRgb(0x1E5AA8);
    inventorModern.primaryLight = QColor::fromRgb(0x5A9DFF);
    inventorModern.background = QColor::fromRgb(0xF5F5F5);
    inventorModern.surface = QColor::fromRgb(0xFFFFFF);
    inventorModern.surfaceHover = QColor::fromRgb(0xF0F0F0);
    inventorModern.surfaceSelected = QColor::fromRgb(0xE3F2FD);
    inventorModern.textPrimary = QColor::fromRgb(0x212121);
    inventorModern.textSecondary = QColor::fromRgb(0x757575);
    inventorModern.textDisabled = QColor::fromRgb(0xBDBDBD);
    inventorModern.textOnPrimary = QColor::fromRgb(0xFFFFFF);
    inventorModern.border = QColor::fromRgb(0xD0D0D0);
    inventorModern.borderLight = QColor::fromRgb(0xE8E8E8);
    inventorModern.borderDark = QColor::fromRgb(0xB0B0B0);
    inventorModern.borderFocus = QColor::fromRgb(0x2B78E4);
    inventorModern.ribbonBackground = QColor::fromRgb(0xFFFFFF);
    inventorModern.ribbonTab = QColor::fromRgb(0xF5F5F5);
    inventorModern.ribbonTabSelected = QColor::fromRgb(0xFFFFFF);
    inventorModern.ribbonTabHover = QColor::fromRgb(0xEEEEEE);
    inventorModern.dockTitle = QColor::fromRgb(0xF0F0F0);
    inventorModern.dockTitleActive = QColor::fromRgb(0xE0E0E0);
    inventorModern.dockContent = QColor::fromRgb(0xFFFFFF);
    inventorModern.statusBar = QColor::fromRgb(0xFAFAFA);
    inventorModern.statusBarBorder = QColor::fromRgb(0xD0D0D0);
    inventorModern.button = QColor::fromRgb(0xFFFFFF);
    inventorModern.buttonHover = QColor::fromRgb(0xF5F5F5);
    inventorModern.buttonPressed = QColor::fromRgb(0xE0E0E0);
    inventorModern.buttonPrimary = QColor::fromRgb(0x2B78E4);
    inventorModern.buttonPrimaryHover = QColor::fromRgb(0x1E5AA8);
    inventorModern.inputBackground = QColor::fromRgb(0xFFFFFF);
    inventorModern.inputBorder = QColor::fromRgb(0xD0D0D0);
    inventorModern.inputBorderFocus = QColor::fromRgb(0x2B78E4);
    inventorModern.treeBackground = QColor::fromRgb(0xFFFFFF);
    inventorModern.treeItemHover = QColor::fromRgb(0xF5F5F5);
    inventorModern.treeItemSelected = QColor::fromRgb(0xE3F2FD);
    inventorModern.scrollbar = QColor::fromRgb(0xE0E0E0);
    inventorModern.scrollbarHandle = QColor::fromRgb(0xC0C0C0);
    inventorModern.scrollbarHandleHover = QColor::fromRgb(0xA0A0A0);
    themes_["InventorModern"] = inventorModern;
    
    // Dark Theme (optional)
    ColorPalette dark;
    dark.primary = QColor::fromRgb(0x64B5F6);
    dark.primaryDark = QColor::fromRgb(0x42A5F5);
    dark.primaryLight = QColor::fromRgb(0x90CAF9);
    dark.background = QColor::fromRgb(0x1E1E1E);
    dark.surface = QColor::fromRgb(0x2D2D2D);
    dark.surfaceHover = QColor::fromRgb(0x3D3D3D);
    dark.surfaceSelected = QColor::fromRgb(0x1E3A5F);
    dark.textPrimary = QColor::fromRgb(0xFFFFFF);
    dark.textSecondary = QColor::fromRgb(0xB0B0B0);
    dark.textDisabled = QColor::fromRgb(0x606060);
    dark.textOnPrimary = QColor::fromRgb(0x000000);
    dark.border = QColor::fromRgb(0x404040);
    dark.borderLight = QColor::fromRgb(0x303030);
    dark.borderDark = QColor::fromRgb(0x505050);
    dark.borderFocus = QColor::fromRgb(0x64B5F6);
    dark.ribbonBackground = QColor::fromRgb(0x2D2D2D);
    dark.ribbonTab = QColor::fromRgb(0x252525);
    dark.ribbonTabSelected = QColor::fromRgb(0x2D2D2D);
    dark.ribbonTabHover = QColor::fromRgb(0x353535);
    dark.dockTitle = QColor::fromRgb(0x252525);
    dark.dockTitleActive = QColor::fromRgb(0x353535);
    dark.dockContent = QColor::fromRgb(0x2D2D2D);
    dark.statusBar = QColor::fromRgb(0x252525);
    dark.statusBarBorder = QColor::fromRgb(0x404040);
    dark.button = QColor::fromRgb(0x2D2D2D);
    dark.buttonHover = QColor::fromRgb(0x3D3D3D);
    dark.buttonPressed = QColor::fromRgb(0x1D1D1D);
    dark.buttonPrimary = QColor::fromRgb(0x64B5F6);
    dark.buttonPrimaryHover = QColor::fromRgb(0x42A5F5);
    dark.inputBackground = QColor::fromRgb(0x2D2D2D);
    dark.inputBorder = QColor::fromRgb(0x404040);
    dark.inputBorderFocus = QColor::fromRgb(0x64B5F6);
    dark.treeBackground = QColor::fromRgb(0x2D2D2D);
    dark.treeItemHover = QColor::fromRgb(0x3D3D3D);
    dark.treeItemSelected = QColor::fromRgb(0x1E3A5F);
    dark.scrollbar = QColor::fromRgb(0x404040);
    dark.scrollbarHandle = QColor::fromRgb(0x606060);
    dark.scrollbarHandleHover = QColor::fromRgb(0x808080);
    themes_["Dark"] = dark;
}

void ThemeManager::setTheme(const QString& themeName) {
    if (themes_.contains(themeName)) {
        currentTheme_ = themeName;
        updatePalette(themeName);
    }
}

QString ThemeManager::getCurrentTheme() const {
    return currentTheme_;
}

QStringList ThemeManager::getAvailableThemes() const {
    return themes_.keys();
}

void ThemeManager::updatePalette(const QString& themeName) {
    palette_ = themes_[themeName];
    
    // Apply custom colors if any
    for (auto it = customColors_.constBegin(); it != customColors_.constEnd(); ++it) {
        // Custom colors override theme colors
        // This would need reflection or a map to work properly
    }
}

QColor ThemeManager::getColor(const QString& colorName) const {
    // Map color names to palette members
    if (colorName == "primary") return palette_.primary;
    if (colorName == "primaryDark") return palette_.primaryDark;
    if (colorName == "primaryLight") return palette_.primaryLight;
    if (colorName == "background") return palette_.background;
    if (colorName == "surface") return palette_.surface;
    if (colorName == "textPrimary") return palette_.textPrimary;
    if (colorName == "textSecondary") return palette_.textSecondary;
    if (colorName == "border") return palette_.border;
    if (colorName == "borderFocus") return palette_.borderFocus;
    
    // Check custom colors
    if (customColors_.contains(colorName)) {
        return customColors_[colorName];
    }
    
    return QColor(); // Invalid color
}

QString ThemeManager::generateStylesheet() const {
    QString style;
    QTextStream stream(&style);
    
    stream << "/* Hydra CAD – Inventor-style theme, modern (no XP look) */\n\n";
    
    // Global: modern font, no flat gray
    stream << "QWidget {\n";
    stream << "    font-family: \"Segoe UI\", \"SF Pro Display\", \"Helvetica Neue\", system-ui, sans-serif;\n";
    stream << "    font-size: 13px;\n";
    stream << "}\n\n";
    
    // Dialogs: clean, rounded, elevated
    stream << "QDialog {\n";
    stream << "    background: " << palette_.surface.name() << ";\n";
    stream << "    border: 1px solid " << palette_.border.name() << ";\n";
    stream << "    border-radius: 8px;\n";
    stream << "}\n\n";
    
    // Progress bar: modern, rounded
    stream << "QProgressBar {\n";
    stream << "    background: " << palette_.scrollbar.name() << ";\n";
    stream << "    border: none;\n";
    stream << "    border-radius: 6px;\n";
    stream << "    text-align: center;\n";
    stream << "    min-height: 10px;\n";
    stream << "}\n\n";
    stream << "QProgressBar::chunk {\n";
    stream << "    background: " << palette_.primary.name() << ";\n";
    stream << "    border-radius: 6px;\n";
    stream << "}\n\n";
    
    // Checkboxes
    stream << "QCheckBox {\n";
    stream << "    color: " << palette_.textPrimary.name() << ";\n";
    stream << "    spacing: 8px;\n";
    stream << "}\n\n";
    
    // Main Window
    stream << "QMainWindow {\n";
    stream << "    background-color: " << palette_.background.name() << ";\n";
    stream << "}\n\n";
    
    // Ribbon – Inventor-style tabs
    stream << "QTabWidget::pane {\n";
    stream << "    border: 1px solid " << palette_.border.name() << ";\n";
    stream << "    background: " << palette_.ribbonBackground.name() << ";\n";
    stream << "    top: -1px;\n";
    stream << "    border-radius: 0 4px 4px 4px;\n";
    stream << "}\n\n";
    
    stream << "QTabBar::tab {\n";
    stream << "    background: " << palette_.ribbonTab.name() << ";\n";
    stream << "    color: " << palette_.textPrimary.name() << ";\n";
    stream << "    padding: 10px 18px;\n";
    stream << "    margin-right: 2px;\n";
    stream << "    border: 1px solid " << palette_.borderLight.name() << ";\n";
    stream << "    border-bottom: none;\n";
    stream << "    border-top-left-radius: 6px;\n";
    stream << "    border-top-right-radius: 6px;\n";
    stream << "}\n\n";
    
    stream << "QTabBar::tab:hover {\n";
    stream << "    background: " << palette_.ribbonTabHover.name() << ";\n";
    stream << "}\n\n";
    
    stream << "QTabBar::tab:selected {\n";
    stream << "    background: " << palette_.ribbonTabSelected.name() << ";\n";
    stream << "    border-bottom: 2px solid " << palette_.primary.name() << ";\n";
    stream << "    font-weight: 600;\n";
    stream << "}\n\n";
    
    // Dock Widgets
    stream << "QDockWidget {\n";
    stream << "    titlebar-close-icon: none;\n";
    stream << "    titlebar-normal-icon: none;\n";
    stream << "    border: 1px solid " << palette_.border.name() << ";\n";
    stream << "}\n\n";
    
    stream << "QDockWidget::title {\n";
    stream << "    background: " << palette_.dockTitle.name() << ";\n";
    stream << "    padding: 6px 8px;\n";
    stream << "    border-bottom: 1px solid " << palette_.border.name() << ";\n";
    stream << "    font-weight: 600;\n";
    stream << "    color: " << palette_.textPrimary.name() << ";\n";
    stream << "}\n\n";
    
    stream << "QDockWidget::title:hover {\n";
    stream << "    background: " << palette_.dockTitleActive.name() << ";\n";
    stream << "}\n\n";
    
    // Buttons – modern, rounded (no flat XP look)
    stream << "QPushButton {\n";
    stream << "    background: " << palette_.button.name() << ";\n";
    stream << "    color: " << palette_.textPrimary.name() << ";\n";
    stream << "    border: 1px solid " << palette_.border.name() << ";\n";
    stream << "    border-radius: 6px;\n";
    stream << "    padding: 8px 16px;\n";
    stream << "    min-height: 28px;\n";
    stream << "    font-weight: 500;\n";
    stream << "}\n\n";
    
    stream << "QPushButton:hover {\n";
    stream << "    background: " << palette_.buttonHover.name() << ";\n";
    stream << "    border-color: " << palette_.borderDark.name() << ";\n";
    stream << "}\n\n";
    
    stream << "QPushButton:pressed {\n";
    stream << "    background: " << palette_.buttonPressed.name() << ";\n";
    stream << "}\n\n";
    
    stream << "QPushButton:default {\n";
    stream << "    background: " << palette_.buttonPrimary.name() << ";\n";
    stream << "    color: " << palette_.textOnPrimary.name() << ";\n";
    stream << "    border-color: " << palette_.primaryDark.name() << ";\n";
    stream << "}\n\n";
    
    stream << "QPushButton:default:hover {\n";
    stream << "    background: " << palette_.buttonPrimaryHover.name() << ";\n";
    stream << "}\n\n";
    
    // Input fields – rounded, clear focus
    stream << "QLineEdit, QTextEdit, QPlainTextEdit {\n";
    stream << "    background: " << palette_.inputBackground.name() << ";\n";
    stream << "    border: 1px solid " << palette_.inputBorder.name() << ";\n";
    stream << "    border-radius: 6px;\n";
    stream << "    padding: 8px 10px;\n";
    stream << "    color: " << palette_.textPrimary.name() << ";\n";
    stream << "    selection-background-color: " << palette_.primary.name() << ";\n";
    stream << "    selection-color: " << palette_.textOnPrimary.name() << ";\n";
    stream << "}\n\n";
    
    stream << "QLineEdit:focus, QTextEdit:focus, QPlainTextEdit:focus {\n";
    stream << "    border: 2px solid " << palette_.borderFocus.name() << ";\n";
    stream << "    padding: 7px 9px;\n";
    stream << "}\n\n";
    
    // Tree/List views – clean panels
    stream << "QTreeView, QListView, QListWidget {\n";
    stream << "    background: " << palette_.treeBackground.name() << ";\n";
    stream << "    border: 1px solid " << palette_.border.name() << ";\n";
    stream << "    border-radius: 6px;\n";
    stream << "    color: " << palette_.textPrimary.name() << ";\n";
    stream << "    outline: none;\n";
    stream << "}\n\n";
    
    stream << "QTreeView::item:hover, QListView::item:hover, QListWidget::item:hover {\n";
    stream << "    background: " << palette_.treeItemHover.name() << ";\n";
    stream << "}\n\n";
    
    stream << "QTreeView::item:selected, QListView::item:selected, QListWidget::item:selected {\n";
    stream << "    background: " << palette_.treeItemSelected.name() << ";\n";
    stream << "    color: " << palette_.textPrimary.name() << ";\n";
    stream << "}\n\n";
    
    // Status bar
    stream << "QStatusBar {\n";
    stream << "    background: " << palette_.statusBar.name() << ";\n";
    stream << "    border-top: 1px solid " << palette_.statusBarBorder.name() << ";\n";
    stream << "    color: " << palette_.textPrimary.name() << ";\n";
    stream << "}\n\n";
    
    stream << "QStatusBar::item {\n";
    stream << "    border: none;\n";
    stream << "}\n\n";
    
    // Scrollbars
    stream << "QScrollBar:vertical {\n";
    stream << "    background: " << palette_.scrollbar.name() << ";\n";
    stream << "    width: 12px;\n";
    stream << "    border: none;\n";
    stream << "    border-radius: 6px;\n";
    stream << "}\n\n";
    
    stream << "QScrollBar::handle:vertical {\n";
    stream << "    background: " << palette_.scrollbarHandle.name() << ";\n";
    stream << "    min-height: 30px;\n";
    stream << "    border-radius: 6px;\n";
    stream << "}\n\n";
    
    stream << "QScrollBar::handle:vertical:hover {\n";
    stream << "    background: " << palette_.scrollbarHandleHover.name() << ";\n";
    stream << "}\n\n";
    
    stream << "QScrollBar:horizontal {\n";
    stream << "    background: " << palette_.scrollbar.name() << ";\n";
    stream << "    height: 12px;\n";
    stream << "    border: none;\n";
    stream << "    border-radius: 6px;\n";
    stream << "}\n\n";
    
    stream << "QScrollBar::handle:horizontal {\n";
    stream << "    background: " << palette_.scrollbarHandle.name() << ";\n";
    stream << "    min-width: 30px;\n";
    stream << "    border-radius: 6px;\n";
    stream << "}\n\n";
    
    stream << "QScrollBar::handle:horizontal:hover {\n";
    stream << "    background: " << palette_.scrollbarHandleHover.name() << ";\n";
    stream << "}\n\n";
    
    // Tool buttons – ribbon-style
    stream << "QToolButton {\n";
    stream << "    background: transparent;\n";
    stream << "    border: 1px solid transparent;\n";
    stream << "    border-radius: 6px;\n";
    stream << "    padding: 6px;\n";
    stream << "}\n\n";
    
    stream << "QToolButton:hover {\n";
    stream << "    background: " << palette_.buttonHover.name() << ";\n";
    stream << "    border-color: " << palette_.border.name() << ";\n";
    stream << "}\n\n";
    
    stream << "QToolButton:pressed {\n";
    stream << "    background: " << palette_.buttonPressed.name() << ";\n";
    stream << "}\n\n";
    
    // Combo boxes – rounded
    stream << "QComboBox {\n";
    stream << "    background: " << palette_.inputBackground.name() << ";\n";
    stream << "    border: 1px solid " << palette_.inputBorder.name() << ";\n";
    stream << "    border-radius: 6px;\n";
    stream << "    padding: 8px 10px;\n";
    stream << "    color: " << palette_.textPrimary.name() << ";\n";
    stream << "}\n\n";
    
    stream << "QComboBox:hover {\n";
    stream << "    border-color: " << palette_.borderDark.name() << ";\n";
    stream << "}\n\n";
    
    stream << "QComboBox:focus {\n";
    stream << "    border: 2px solid " << palette_.borderFocus.name() << ";\n";
    stream << "    padding: 7px 9px;\n";
    stream << "}\n\n";
    
    stream << "QComboBox::drop-down {\n";
    stream << "    border: none;\n";
    stream << "    width: 24px;\n";
    stream << "}\n\n";
    
    // Labels
    stream << "QLabel {\n";
    stream << "    color: " << palette_.textPrimary.name() << ";\n";
    stream << "}\n\n";
    
    // Group boxes – Inventor-style collapsible sections (flat, rounded)
    stream << "QGroupBox {\n";
    stream << "    font-weight: 600;\n";
    stream << "    color: " << palette_.textPrimary.name() << ";\n";
    stream << "    border: 1px solid " << palette_.borderLight.name() << ";\n";
    stream << "    border-radius: 6px;\n";
    stream << "    margin-top: 10px;\n";
    stream << "    padding: 12px 10px 8px 10px;\n";
    stream << "    background: " << palette_.surface.name() << ";\n";
    stream << "}\n\n";
    stream << "QGroupBox::title {\n";
    stream << "    subcontrol-origin: margin;\n";
    stream << "    subcontrol-position: top left;\n";
    stream << "    left: 10px;\n";
    stream << "    padding: 0 6px;\n";
    stream << "    color: " << palette_.textPrimary.name() << ";\n";
    stream << "}\n\n";
    
    // Ribbon group frame (Inventor-style vertical group)
    stream << "QFrame#ribbonGroup {\n";
    stream << "    border: 1px solid " << palette_.borderLight.name() << ";\n";
    stream << "    border-radius: 6px;\n";
    stream << "    background: transparent;\n";
    stream << "}\n\n";
    stream << "QFrame#ribbonGroup:hover {\n";
    stream << "    background: " << palette_.surfaceHover.name() << ";\n";
    stream << "}\n\n";
    stream << "QLabel#ribbonGroupLabel {\n";
    stream << "    color: " << palette_.textSecondary.name() << ";\n";
    stream << "    font-size: 11px;\n";
    stream << "    font-weight: 600;\n";
    stream << "}\n\n";
    
    return style;
}

void ThemeManager::applyStylesheet(QWidget* widget) const {
    if (widget) {
        widget->setStyleSheet(generateStylesheet());
    }
}

void ThemeManager::saveSettings(QSettings& settings) const {
    settings.setValue("theme/current", currentTheme_);
    // Save custom colors if needed
}

void ThemeManager::loadSettings(QSettings& settings) {
    QString theme = settings.value("theme/current", "InventorModern").toString();
    setTheme(theme);
}

void ThemeManager::setCustomColor(const QString& colorName, const QColor& color) {
    customColors_[colorName] = color;
}

void ThemeManager::resetToDefault() {
    customColors_.clear();
    updatePalette(currentTheme_);
}

}  // namespace ui
}  // namespace cad
