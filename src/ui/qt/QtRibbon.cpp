#include "QtRibbon.h"

#include <QGroupBox>
#include <QLabel>
#include <QToolButton>
#include <QVBoxLayout>
#include <QKeySequence>
#include <QIcon>
#include <QFile>
#include <QDir>

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
        {tr("Sketch"),
         {{tr("Create"), {"Line", "Rectangle", "Circle", "Arc"}},
          {tr("Constraints"), {"Constraint"}}},
         tr("Create sketches and constraints")},
        {tr("Part"),
         {{tr("Features"), {"Extrude", "Revolve", "Loft", "Hole", "Fillet"}},
          {tr("Sheet Metal"), {"Flange", "Bend", "Unfold", "Refold"}},
          {tr("Pattern"), {"RectangularPattern", "CircularPattern", "CurvePattern"}},
          {tr("Direct"), {"DirectEdit", "Freeform"}}},
         tr("Create and edit parts")},
        {tr("Assembly"),
         {{tr("Assemble"), {"LoadAssembly", "Place", "Mate", "Flush", "Angle", "Pattern"}},
          {tr("Routing"), {"RigidPipe", "FlexibleHose", "BentTube"}},
          {tr("Simplify"), {"Simplify"}}},
         tr("Assemble components and mates")},
        {tr("Drawing"),
         {{tr("Views"), {"BaseView", "Section", "Dimension", "PartsList"}}},
         tr("Create drawings and annotations")},
        {tr("Inspect"),
         {{tr("Analysis"), {"Measure", "Interference", "SectionAnalysis"}},
          {tr("Simulation"), {"Simulation", "StressAnalysis"}}},
         tr("Inspect and simulate models")},
        {tr("Manage"),
         {{tr("Parameters"), {"Parameters", "Styles", "AddIns"}},
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
        {"Revolve", tr("Revolve")},
        {"Loft", tr("Loft")},
        {"Hole", tr("Hole")},
        {"Fillet", tr("Fillet")},
        {"Flange", tr("Flange")},
        {"Bend", tr("Bend")},
        {"Unfold", tr("Unfold")},
        {"Refold", tr("Refold")},
        {"RectangularPattern", tr("Rectangular Pattern")},
        {"CircularPattern", tr("Circular Pattern")},
        {"CurvePattern", tr("Curve Pattern")},
        {"DirectEdit", tr("Direct Edit")},
        {"Freeform", tr("Freeform")},
        {"LoadAssembly", tr("Load Assembly")},
        {"Place", tr("Place")},
        {"Mate", tr("Mate")},
        {"Flush", tr("Flush")},
        {"Angle", tr("Angle")},
        {"Pattern", tr("Pattern")},
        {"RigidPipe", tr("Rigid Pipe")},
        {"FlexibleHose", tr("Flexible Hose")},
        {"BentTube", tr("Bent Tube")},
        {"Simplify", tr("Simplify")},
        {"BaseView", tr("Base View")},
        {"Section", tr("Section")},
        {"Dimension", tr("Dimension")},
        {"PartsList", tr("Parts List")},
        {"Measure", tr("Measure")},
        {"Interference", tr("Interference")},
        {"SectionAnalysis", tr("Section Analysis")},
        {"Simulation", tr("Simulation")},
        {"StressAnalysis", tr("Stress Analysis")},
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
        {"Animation", tr("Animation")}
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
        {"Fillet", {"format-line-spacing", "tools"}},
        {"Chamfer", {"format-line-spacing", "tools"}},
        {"Shell", {"object-flip-horizontal", "view-3d"}},
        {"Mirror", {"object-flip-horizontal", "transform-mirror"}},
        {"RectangularPattern", {"view-grid", "transform-duplicate"}},
        {"CircularPattern", {"view-refresh", "transform-duplicate"}},
        {"CurvePattern", {"draw-curve", "transform-duplicate"}},
        {"DirectEdit", {"document-edit", "tools"}},
        {"Freeform", {"draw-freehand", "tools"}},
        {"Flange", {"tools", "view-3d"}},
        {"Bend", {"view-refresh", "tools"}},
        {"Unfold", {"view-refresh", "tools"}},
        {"Refold", {"view-refresh", "tools"}},

        // Assembly
        {"LoadAssembly", {"document-open", "folder-open"}},
        {"Place", {"object-position", "transform-move"}},
        {"Mate", {"link", "insert-link"}},
        {"Flush", {"align-horizontal-center", "align-vertical-center"}},
        {"Angle", {"format-text-italic", "tools"}},
        {"Pattern", {"view-grid", "transform-duplicate"}},
        {"RigidPipe", {"network-wired", "view-3d"}},
        {"FlexibleHose", {"network-wired", "view-3d"}},
        {"BentTube", {"network-wired", "view-3d"}},
        {"Simplify", {"edit-clear", "tools"}},

        // Drawing
        {"BaseView", {"document-preview", "x-office-document"}},
        {"Section", {"document-properties", "view-split-left-right"}},
        {"Dimension", {"format-indent-more", "draw-text"}},
        {"PartsList", {"view-list-details", "x-office-spreadsheet"}},

        // Inspect
        {"Measure", {"measure", "zoom-original"}},
        {"Interference", {"dialog-warning", "process-stop"}},
        {"SectionAnalysis", {"document-properties", "view-split-left-right"}},
        {"Simulation", {"media-playback-start", "system-run"}},
        {"StressAnalysis", {"dialog-warning", "system-run"}},

        // Manage
        {"Parameters", {"preferences-other", "preferences-system"}},
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
        {"Fillet", ":/icons/part/fillet.svg"},
        {"Chamfer", ":/icons/part/chamfer.svg"},
        {"Shell", ":/icons/part/shell.svg"},
        {"Mirror", ":/icons/part/mirror.svg"},
        {"RectangularPattern", ":/icons/part/pattern_rect.svg"},
        {"CircularPattern", ":/icons/part/pattern_circ.svg"},
        
        // Assembly icons
        {"Place", ":/icons/assembly/place.svg"},
        {"Mate", ":/icons/assembly/mate.svg"},
        {"Constraint", ":/icons/assembly/constraint.svg"},
        {"Pattern", ":/icons/assembly/pattern.svg"},
        
        // Drawing icons
        {"BaseView", ":/icons/drawing/base_view.svg"},
        {"Section", ":/icons/drawing/section.svg"},
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
        {"ZoomOut", ":/icons/general/zoom_out.svg"}
    };
    
    if (iconMap.contains(commandId)) {
        return iconMap[commandId];
    }
    return QString();
}

QWidget* QtRibbon::buildCommandTab(const QString& title,
                                   const QList<QPair<QString, QStringList>>& groups) {
    QWidget* page = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->addWidget(new QLabel(title));
    for (const auto& group : groups) {
        layout->addWidget(buildGroup(group.first, group.second));
    }
    layout->addStretch();
    return page;
}

QWidget* QtRibbon::buildGroup(const QString& name, const QStringList& command_ids) {
    QGroupBox* group = new QGroupBox(name, this);
    QVBoxLayout* layout = new QVBoxLayout(group);
    for (const auto& id : command_ids) {
        QAction* action = registerAction(id, id);
        QToolButton* button = new QToolButton(group);
        button->setDefaultAction(action);
        button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        layout->addWidget(button);
    }
    group->setLayout(layout);
    return group;
}

}  // namespace ui
}  // namespace cad
