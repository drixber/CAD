#include "RibbonLayout.h"

namespace cad {
namespace ui {

void RibbonLayout::configureDefaultTabs() {
    tabs_.clear();
    commands_.clear();
    tabs_.push_back("Sketch");
    tabs_.push_back("Part");
    tabs_.push_back("Assembly");
    tabs_.push_back("Drawing");
    tabs_.push_back("Inspect");
    tabs_.push_back("Manage");
    tabs_.push_back("View");

    commands_["Sketch"] = {"Line", "Rectangle", "Circle", "Arc", "Constraint"};
    commands_["Part"] = {"Extrude", "Revolve", "Loft", "Hole", "HoleThroughAll", "Fillet", "Chamfer", "Shell", "Mirror",
                         "Flange", "Bend", "Unfold", "Refold", "Punch", "Bead",
                         "SheetMetalRules", "ExportFlatDXF",
                         "RectangularPattern", "CircularPattern", "CurvePattern", "FacePattern",
                         "DirectEdit", "Freeform"};
    commands_["Assembly"] = {"LoadAssembly", "Place", "Mate", "Flush", "Angle", "Parallel", "Distance", "Pattern", "ExplosionView",
                             "RigidPipe", "FlexibleHose", "BentTube", "RouteBOM",
                             "Weld", "WeldBOM", "Simplify"};
    commands_["Drawing"] = {"BaseView", "Section", "DetailView", "Dimension", "PartsList"};
    commands_["Inspect"] = {"Measure", "Interference", "SectionAnalysis",
                            "Simulation", "StressAnalysis", "ExportFEAReport", "ExportMotionReport"};
    commands_["Manage"] = {"Parameters", "iLogic", "Styles", "AddIns", "Import", "Export", "ExportRFA", "MbdNote"};
    commands_["View"] = {"Visibility", "Appearance", "Environment",
                         "Illustration", "Rendering", "Animation"};
}

const std::vector<std::string>& RibbonLayout::tabs() const {
    return tabs_;
}

const std::unordered_map<std::string, std::vector<std::string>>& RibbonLayout::commands() const {
    return commands_;
}

}  // namespace ui
}  // namespace cad
