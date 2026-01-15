#include "QtRibbon.h"

#include <QGroupBox>
#include <QLabel>
#include <QToolButton>
#include <QVBoxLayout>

namespace cad {
namespace ui {

QtRibbon::QtRibbon(QWidget* parent) : QTabWidget(parent) {
    registerDefaultActions();

    addTab(buildCommandTab(tr("Sketch"),
                           {{tr("Create"),
                             {"Line", "Rectangle", "Circle", "Arc"}},
                            {tr("Constraints"),
                             {"Constraint"}}}),
           tr("Sketch"));
    addTab(buildCommandTab(tr("Part"),
                           {{tr("Features"),
                             {"Extrude", "Revolve", "Loft", "Hole", "Fillet"}},
                            {tr("Sheet Metal"),
                             {"Flange", "Bend", "Unfold", "Refold"}},
                            {tr("Pattern"),
                             {"RectangularPattern", "CircularPattern", "CurvePattern"}},
                            {tr("Direct"),
                             {"DirectEdit", "Freeform"}}}),
           tr("Part"));
    addTab(buildCommandTab(tr("Assembly"),
                           {{tr("Assemble"),
                             {"Place", "Mate", "Flush", "Angle", "Pattern"}},
                            {tr("Routing"),
                             {"RigidPipe", "FlexibleHose", "BentTube"}},
                            {tr("Simplify"),
                             {"Simplify"}}}),
           tr("Assembly"));
    addTab(buildCommandTab(tr("Drawing"),
                           {{tr("Views"),
                             {"BaseView", "Section", "Dimension", "PartsList"}}}),
           tr("Drawing"));
    addTab(buildCommandTab(tr("Inspect"),
                           {{tr("Analysis"),
                             {"Measure", "Interference", "SectionAnalysis"}},
                            {tr("Simulation"),
                             {"Simulation", "StressAnalysis"}}}),
           tr("Inspect"));
    addTab(buildCommandTab(tr("Manage"),
                           {{tr("Parameters"),
                             {"Parameters", "Styles", "AddIns"}},
                            {tr("Interop"),
                             {"Import", "Export", "ExportRFA"}},
                            {tr("MBD"),
                             {"MbdNote"}}}),
           tr("Manage"));
    addTab(buildCommandTab(tr("View"),
                           {{tr("Display"),
                             {"Visibility", "Appearance", "Environment"}},
                            {tr("Render"),
                             {"Illustration", "Rendering", "Animation"}}}),
           tr("View"));
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
        registerAction(item.first, item.second);
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
    connect(action, &QAction::triggered, this, [this, id]() {
        if (command_handler_) {
            command_handler_(id);
        }
    });
    actions_.insert({key, action});
    return action;
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
