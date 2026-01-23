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
    QString iconPath = getIconPath(id);
    if (!iconPath.isEmpty() && QFile::exists(iconPath)) {
        action->setIcon(QIcon(iconPath));
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
