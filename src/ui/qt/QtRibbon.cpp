#include "QtRibbon.h"

#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QToolButton>
#include <QVBoxLayout>
#include <QKeySequence>
#include <QIcon>
#include <QFile>
#include <QDir>
#include <QFrame>
#include <QSize>
#include <QLineEdit>
#include <QTabBar>

namespace cad {
namespace ui {

QtRibbon::QtRibbon(QWidget* parent) : QTabWidget(parent) {
    registerDefaultActions();

    struct TabConfig {
        QString name;
        QList<QPair<QString, QStringList>> groups;
        QString tooltip;
    };

    const TabConfig tabs[] = {
        {tr("File"),
         {{tr("Document"), {"New", "Open", "Save"}},
          {tr("Interop"), {"Import", "Export"}}},
         tr("New, open, save, import, export")},
        {tr("Sketch"),
         {{tr("Create"), {"Line", "Rectangle", "Circle", "Arc"}},
          {tr("Constraints"), {"Constraint"}}},
         tr("Create sketches and constraints")},
        {tr("Part"),
         {{tr("Features"), {"Extrude", "Revolve", "Loft", "Hole", "HoleThroughAll", "Fillet", "Chamfer", "Shell", "Mirror"}},
          {tr("Sheet Metal"), {"Flange", "Bend", "Unfold", "Refold", "Punch", "Bead", "SheetMetalRules", "ExportFlatDXF"}},
          {tr("Pattern"), {"RectangularPattern", "CircularPattern", "CurvePattern", "FacePattern"}},
          {tr("Direct"), {"DirectEdit", "Freeform"}}},
         tr("Create and edit parts")},
        {tr("Assembly"),
         {{tr("Assemble"), {"LoadAssembly", "Place", "Mate", "Flush", "Angle", "Parallel", "Distance", "Pattern"}},
          {tr("Presentation"), {"ExplosionView"}},
          {tr("Routing"), {"RigidPipe", "FlexibleHose", "BentTube", "RouteBOM"}},
          {tr("Welding"), {"Weld", "WeldBOM"}},
          {tr("Simplify"), {"Simplify"}}},
         tr("Assemble components and mates")},
        {tr("Drawing"),
         {{tr("Views"), {"BaseView", "Section", "DetailView", "Dimension", "PartsList"}}},
         tr("Create drawings and annotations")},
        {tr("Inspect"),
         {{tr("Analysis"), {"Measure", "Interference", "SectionAnalysis"}},
          {tr("Simulation"), {"Simulation", "StressAnalysis", "ExportFEAReport", "ExportMotionReport"}}},
         tr("Inspect and simulate models")},
        {tr("Manage"),
         {{tr("Parameters"), {"Parameters", "iLogic", "Styles", "AddIns"}},
          {tr("Interop"), {"Import", "Export", "ExportRFA"}},
          {tr("MBD"), {"MbdNote"}}},
         tr("Manage data and interoperability")},
        {tr("View"),
         {{tr("Display"), {"Visibility", "Appearance", "Environment"}},
          {tr("Render"), {"Illustration", "Rendering", "Animation"}}},
         tr("Control visibility and rendering")},
    };

    for (const auto& tab : tabs) {
        addTab(buildCommandTab(tab.name, tab.groups), tab.name);
        setTabToolTip(count() - 1, tab.tooltip);
    }

    search_line_ = new QLineEdit(this);
    search_line_->setPlaceholderText(tr("Search Help & Commands…"));
    search_line_->setClearButtonEnabled(true);
    search_line_->setMinimumWidth(200);
    search_line_->setMaximumWidth(280);
    search_line_->setObjectName("ribbonSearchLine");

    document_tabs_ = new QTabBar(this);
    document_tabs_->addTab(tr("Document 1"));
    document_tabs_->setExpanding(false);
    document_tabs_->setObjectName("documentTabBar");
}

void QtRibbon::setCommandHandler(const std::function<void(const QString&)>& handler) {
    command_handler_ = handler;
}

void QtRibbon::registerDefaultActions() {
    const QList<QPair<QString, QString>> commands = {
        {"Line", tr("Line")},
        {"Rectangle", tr("Rectangle")},
        {"Circle", tr("Circle")},
        {"Arc", tr("Arc")},
        {"Constraint", tr("Constraint")},
        {"Extrude", tr("Extrude")},
        {"ExtrudeReverse", tr("Extrude (Reverse)")},
        {"ExtrudeBoth", tr("Extrude (Both)")},
        {"Revolve", tr("Revolve")},
        {"Loft", tr("Loft")},
        {"Hole", tr("Hole")},
        {"HoleThroughAll", tr("Hole (Through All)")},
        {"Fillet", tr("Fillet")},
        {"Flange", tr("Flange")},
        {"Bend", tr("Bend")},
        {"Unfold", tr("Unfold")},
        {"Refold", tr("Refold")},
        {"Punch", tr("Punch (Stanzung)")},
        {"Bead", tr("Bead (Sicke)")},
        {"SheetMetalRules", tr("Sheet Metal Rules")},
        {"ExportFlatDXF", tr("Export Flat DXF")},
        {"RectangularPattern", tr("Rectangular Pattern")},
        {"CircularPattern", tr("Circular Pattern")},
        {"CurvePattern", tr("Curve Pattern")},
        {"FacePattern", tr("Face Pattern (Flächenmuster)")},
        {"DirectEdit", tr("Direct Edit")},
        {"Freeform", tr("Freeform")},
        {"LoadAssembly", tr("Load Assembly")},
        {"Place", tr("Place")},
        {"Mate", tr("Mate")},
        {"Flush", tr("Flush")},
        {"Angle", tr("Angle")},
        {"Parallel", tr("Parallel")},
        {"Distance", tr("Distance")},
        {"Pattern", tr("Pattern")},
        {"ExplosionView", tr("Explosion View")},
        {"RigidPipe", tr("Rigid Pipe")},
        {"FlexibleHose", tr("Flexible Hose")},
        {"BentTube", tr("Bent Tube")},
        {"RouteBOM", tr("Route BOM")},
        {"Weld", tr("Weld")},
        {"WeldBOM", tr("Weld BOM")},
        {"Simplify", tr("Simplify")},
        {"BaseView", tr("Base View")},
        {"Section", tr("Section")},
        {"DetailView", tr("Detail View")},
        {"Dimension", tr("Dimension")},
        {"PartsList", tr("Parts List")},
        {"Measure", tr("Measure")},
        {"Interference", tr("Interference")},
        {"SectionAnalysis", tr("Section Analysis")},
        {"Simulation", tr("Simulation")},
        {"StressAnalysis", tr("Stress Analysis")},
        {"ExportFEAReport", tr("Export FEA Report")},
        {"ExportMotionReport", tr("Export Motion Report")},
        {"Parameters", tr("Parameters")},
        {"Styles", tr("Styles")},
        {"AddIns", tr("Add-Ins")},
        {"Import", tr("Import")},
        {"Export", tr("Export")},
        {"ExportRFA", tr("Export RFA")},
        {"MbdNote", tr("MBD Note")},
        {"Visibility", tr("Visibility")},
        {"Appearance", tr("Appearance")},
        {"Environment", tr("Environment")},
        {"Illustration", tr("Illustration")},
        {"Rendering", tr("Rendering")},
        {"Animation", tr("Animation")},
        {"New", tr("New")},
        {"Open", tr("Open")},
        {"Save", tr("Save")},
        {"GetStarted", tr("Get Started")},
        {"Documentation", tr("Documentation")}
    };

    for (const auto& item : commands) {
        QAction* action = registerAction(item.first, item.second);
        if (item.first == "Line") {
            action->setShortcut(QKeySequence("L"));
        } else if (item.first == "Rectangle") {
            action->setShortcut(QKeySequence("R"));
        } else if (item.first == "Circle") {
            action->setShortcut(QKeySequence("C"));
        } else if (item.first == "Extrude") {
            action->setShortcut(QKeySequence("E"));
        } else if (item.first == "Mate") {
            action->setShortcut(QKeySequence("M"));
        } else if (item.first == "LoadAssembly") {
            action->setShortcut(QKeySequence("Shift+L"));
        }
    }
}

QAction* QtRibbon::registerAction(const QString& id, const QString& label) {
    const std::string key = id.toStdString();
    auto found = actions_.find(key);
    if (found != actions_.end()) {
        return found->second;
    }
    QAction* action = new QAction(label, this);
    action->setObjectName(id);
    action->setShortcutVisibleInContextMenu(true);
    
    // Set icon if available
    QIcon icon = getIcon(id);
    if (!icon.isNull()) {
        action->setIcon(icon);
    }
    
    // Enhanced tooltip with shortcut
    QString tooltip = label;
    if (action->shortcut().isEmpty() == false) {
        tooltip += QString(" (%1)").arg(action->shortcut().toString());
    }
    action->setToolTip(tooltip);
    
    connect(action, &QAction::triggered, this, [this, id]() {
        if (command_handler_) {
            command_handler_(id);
        }
    });
    actions_.insert({key, action});
    return action;
}

namespace {
QIcon iconFromThemeNames(const QStringList& names) {
    for (const auto& name : names) {
        if (name.isEmpty()) {
            continue;
        }
        QIcon icon = QIcon::fromTheme(name);
        if (!icon.isNull()) {
            return icon;
        }
    }
    return QIcon();
}
}  // namespace

QIcon QtRibbon::getIcon(const QString& commandId) {
    const QMap<QString, QStringList> themeMap = {
        // Sketch
        {"Line", {"draw-line"}},
        {"Rectangle", {"draw-rectangle"}},
        {"Circle", {"draw-ellipse"}},
        {"Arc", {"draw-arc", "draw-curve"}},
        {"Spline", {"draw-path", "draw-freehand"}},
        {"Text", {"draw-text"}},
        {"Constraint", {"link", "insert-link"}},

        // Part
        {"Extrude", {"transform-move", "view-3d"}},
        {"Revolve", {"object-rotate-right", "view-refresh"}},
        {"Loft", {"transform-scale", "view-3d"}},
        {"Sweep", {"transform-move", "view-3d"}},
        {"Hole", {"insert-object", "edit-cut"}},
        {"HoleThroughAll", {"insert-object", "edit-cut"}},
        {"Fillet", {"format-line-spacing", "tools"}},
        {"Chamfer", {"format-line-spacing", "tools"}},
        {"Shell", {"object-flip-horizontal", "view-3d"}},
        {"Mirror", {"object-flip-horizontal", "transform-mirror"}},
        {"RectangularPattern", {"view-grid", "transform-duplicate"}},
        {"CircularPattern", {"view-refresh", "transform-duplicate"}},
        {"CurvePattern", {"draw-curve", "transform-duplicate"}},
        {"FacePattern", {"view-grid", "transform-duplicate"}},
        {"DirectEdit", {"document-edit", "tools"}},
        {"Freeform", {"draw-freehand", "tools"}},
        {"Flange", {"tools", "view-3d"}},
        {"Bend", {"view-refresh", "tools"}},
        {"Unfold", {"view-refresh", "tools"}},
        {"Refold", {"view-refresh", "tools"}},
        {"Punch", {"edit-cut", "tools"}},
        {"Bead", {"draw-rectangle", "tools"}},
        {"SheetMetalRules", {"preferences-other", "preferences-system"}},
        {"ExportFlatDXF", {"document-export", "document-save"}},

        // Assembly
        {"LoadAssembly", {"document-open", "folder-open"}},
        {"Place", {"object-position", "transform-move"}},
        {"Mate", {"link", "insert-link"}},
        {"Flush", {"align-horizontal-center", "align-vertical-center"}},
        {"Angle", {"format-text-italic", "tools"}},
        {"Parallel", {"align-horizontal-center", "align-vertical-center"}},
        {"Distance", {"measure", "zoom-original"}},
        {"Pattern", {"view-grid", "transform-duplicate"}},
        {"ExplosionView", {"view-fullscreen", "transform-move"}},
        {"RigidPipe", {"network-wired", "view-3d"}},
        {"FlexibleHose", {"network-wired", "view-3d"}},
        {"BentTube", {"network-wired", "view-3d"}},
        {"RouteBOM", {"view-list-details", "x-office-spreadsheet"}},
        {"Weld", {"edit-copy", "tools"}},
        {"WeldBOM", {"view-list-details", "x-office-spreadsheet"}},
        {"Simplify", {"edit-clear", "tools"}},

        // Drawing
        {"BaseView", {"document-preview", "x-office-document"}},
        {"Section", {"document-properties", "view-split-left-right"}},
        {"DetailView", {"zoom-in", "view-split-left-right"}},
        {"Dimension", {"format-indent-more", "draw-text"}},
        {"PartsList", {"view-list-details", "x-office-spreadsheet"}},

        // Inspect
        {"Measure", {"measure", "zoom-original"}},
        {"Interference", {"dialog-warning", "process-stop"}},
        {"SectionAnalysis", {"document-properties", "view-split-left-right"}},
        {"Simulation", {"media-playback-start", "system-run"}},
        {"StressAnalysis", {"dialog-warning", "system-run"}},
        {"ExportFEAReport", {"document-export", "document-save"}},
        {"ExportMotionReport", {"document-export", "media-playback-start"}},

        // Manage
        {"Parameters", {"preferences-other", "preferences-system"}},
        {"iLogic", {"preferences-other", "applications-system"}},
        {"Styles", {"preferences-desktop-theme", "preferences-system"}},
        {"AddIns", {"application-x-addon", "preferences-system"}},
        {"ExportRFA", {"document-export", "document-save"}},
        {"MbdNote", {"insert-text", "draw-text"}},

        // View
        {"Visibility", {"view-visible", "view-refresh"}},
        {"Appearance", {"preferences-desktop-theme", "preferences-system"}},
        {"Environment", {"preferences-system", "applications-system"}},
        {"Illustration", {"media-playback-start", "applications-graphics"}},
        {"Rendering", {"applications-graphics", "view-preview"}},
        {"Animation", {"media-playback-start", "applications-multimedia"}},

        // File
        {"New", {"document-new"}},
        {"Open", {"document-open"}},
        {"Save", {"document-save"}},
        {"Export", {"document-export"}},
        {"Import", {"document-import"}},

        // General
        {"Undo", {"edit-undo"}},
        {"Redo", {"edit-redo"}},
        {"ZoomFit", {"zoom-fit-best"}},
        {"ZoomIn", {"zoom-in"}},
        {"ZoomOut", {"zoom-out"}},

        // View mode
        {"Shaded", {"view-3d", "applications-graphics"}},
        {"Wireframe", {"view-grid", "applications-graphics"}},
        {"HiddenLine", {"view-filter", "applications-graphics"}},
        {"SectionPlane", {"view-split-left-right", "applications-graphics"}}
    };

    const QStringList defaultThemeNames = {
        "applications-graphics",
        "applications-engineering",
        "tools",
        "preferences-system"
    };

    QIcon themeIcon = iconFromThemeNames(themeMap.value(commandId, defaultThemeNames));
    if (!themeIcon.isNull()) {
        return themeIcon;
    }

    QString iconPath = getIconPath(commandId);
    if (!iconPath.isEmpty() && QFile::exists(iconPath)) {
        return QIcon(iconPath);
    }

    return QIcon();
}

QString QtRibbon::getIconPath(const QString& commandId) {
    // Map command IDs to icon paths
    QMap<QString, QString> iconMap = {
        // Sketch icons
        {"Line", ":/icons/sketch/line.svg"},
        {"Rectangle", ":/icons/sketch/rectangle.svg"},
        {"Circle", ":/icons/sketch/circle.svg"},
        {"Arc", ":/icons/sketch/arc.svg"},
        {"Spline", ":/icons/sketch/spline.svg"},
        {"Text", ":/icons/sketch/text.svg"},
        
        // Part icons
        {"Extrude", ":/icons/part/extrude.svg"},
        {"Revolve", ":/icons/part/revolve.svg"},
        {"Loft", ":/icons/part/loft.svg"},
        {"Sweep", ":/icons/part/sweep.svg"},
        {"Hole", ":/icons/part/hole.svg"},
        {"HoleThroughAll", ":/icons/part/hole.svg"},
        {"Fillet", ":/icons/part/fillet.svg"},
        {"Chamfer", ":/icons/part/chamfer.svg"},
        {"Shell", ":/icons/part/shell.svg"},
        {"Mirror", ":/icons/part/mirror.svg"},
        {"RectangularPattern", ":/icons/part/pattern_rect.svg"},
        {"CircularPattern", ":/icons/part/pattern_circ.svg"},
        {"CurvePattern", ":/icons/part/pattern_rect.svg"},
        {"FacePattern", ":/icons/part/pattern_rect.svg"},
        
        // Assembly icons
        {"Place", ":/icons/assembly/place.svg"},
        {"Mate", ":/icons/assembly/mate.svg"},
        {"Constraint", ":/icons/assembly/constraint.svg"},
        {"Pattern", ":/icons/assembly/pattern.svg"},
        {"ExplosionView", ":/icons/assembly/pattern.svg"},
        
        // Drawing icons
        {"BaseView", ":/icons/drawing/base_view.svg"},
        {"Section", ":/icons/drawing/section.svg"},
        {"DetailView", ":/icons/drawing/section.svg"},
        {"Dimension", ":/icons/drawing/dimension.svg"},
        {"PartsList", ":/icons/drawing/bom.svg"},
        
        // View icons
        {"Shaded", ":/icons/view/shaded.svg"},
        {"Wireframe", ":/icons/view/wireframe.svg"},
        {"HiddenLine", ":/icons/view/hidden_line.svg"},
        {"SectionPlane", ":/icons/view/section_plane.svg"},
        {"Measure", ":/icons/view/measure.svg"},
        
        // File icons
        {"New", ":/icons/file/new.svg"},
        {"Open", ":/icons/file/open.svg"},
        {"Save", ":/icons/file/save.svg"},
        {"Export", ":/icons/file/export.svg"},
        {"Import", ":/icons/file/import.svg"},
        
        // General icons
        {"Undo", ":/icons/general/undo.svg"},
        {"Redo", ":/icons/general/redo.svg"},
        {"ZoomFit", ":/icons/general/zoom_fit.svg"},
        {"ZoomIn", ":/icons/general/zoom_in.svg"},
        {"ZoomOut", ":/icons/general/zoom_out.svg"},
        {"GetStarted", ":/icons/file/new.svg"},
        {"Documentation", ":/icons/file/export.svg"}
    };
    
    if (iconMap.contains(commandId)) {
        return iconMap[commandId];
    }
    return QString();
}

QWidget* QtRibbon::buildCommandTab(const QString& title,
                                   const QList<QPair<QString, QStringList>>& groups) {
    Q_UNUSED(title);
    QWidget* page = new QWidget(this);
    QHBoxLayout* layout = new QHBoxLayout(page);
    layout->setContentsMargins(8, 6, 8, 6);
    layout->setSpacing(4);
    for (const auto& group : groups) {
        layout->addWidget(buildGroup(group.first, group.second));
    }
    layout->addStretch();
    return page;
}

QWidget* QtRibbon::buildGroup(const QString& name, const QStringList& command_ids) {
    QFrame* frame = new QFrame(this);
    frame->setObjectName("ribbonGroup");
    frame->setMinimumHeight(100);
    frame->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
    QVBoxLayout* layout = new QVBoxLayout(frame);
    layout->setContentsMargins(8, 6, 8, 6);
    layout->setSpacing(4);
    QLabel* groupLabel = new QLabel(name, frame);
    groupLabel->setObjectName("ribbonGroupLabel");
    layout->addWidget(groupLabel, 0, Qt::AlignHCenter);
    // Every button (all tabs: Sketch, Part, Assembly, Drawing, Inspect, Manage, View) gets explicit clicked connect
    for (const auto& id : command_ids) {
        if (id == "Extrude") {
            QToolButton* dropdown = new QToolButton(frame);
            QAction* mainAction = registerAction("Extrude", tr("Extrude"));
            dropdown->setDefaultAction(mainAction);
            dropdown->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
            dropdown->setIconSize(QSize(32, 32));
            dropdown->setMinimumSize(56, 52);
            dropdown->setPopupMode(QToolButton::MenuButtonPopup);
            connect(dropdown, &QToolButton::clicked, this, [this]() {
                if (command_handler_) command_handler_(QStringLiteral("Extrude"));
            });
            QMenu* menu = new QMenu(dropdown);
            QAction* actNormal = registerAction("Extrude", tr("Normal"));
            QAction* actReverse = registerAction("ExtrudeReverse", tr("Reverse"));
            QAction* actBoth = registerAction("ExtrudeBoth", tr("Both"));
            menu->addAction(actNormal);
            menu->addAction(actReverse);
            menu->addAction(actBoth);
            dropdown->setMenu(menu);
            connect(actNormal, &QAction::triggered, this, [this]() {
                if (command_handler_) command_handler_(QStringLiteral("Extrude"));
            });
            connect(actReverse, &QAction::triggered, this, [this]() {
                if (command_handler_) command_handler_(QStringLiteral("ExtrudeReverse"));
            });
            connect(actBoth, &QAction::triggered, this, [this]() {
                if (command_handler_) command_handler_(QStringLiteral("ExtrudeBoth"));
            });
            layout->addWidget(dropdown, 0, Qt::AlignHCenter);
            continue;
        }
        QAction* action = registerAction(id, id);
        QToolButton* button = new QToolButton(frame);
        button->setDefaultAction(action);
        button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        button->setIconSize(QSize(32, 32));
        button->setMinimumSize(56, 52);
        connect(button, &QToolButton::clicked, this, [this, id]() {
            if (command_handler_) {
                command_handler_(id);
            }
        });
        layout->addWidget(button, 0, Qt::AlignHCenter);
    }
    frame->setLayout(layout);
    return frame;
}

}  // namespace ui
}  // namespace cad
