#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>

#include "core/Modeler/Modeler.h"
#include "core/Modeler/Part.h"
#include "core/Modeler/Sketch.h"
#include "core/Modeler/Assembly.h"
#include "core/Modeler/Transform.h"
#include "core/undo/UndoStack.h"
#include "core/undo/Command.h"
#include "interop/ImportExportService.h"
#include "modules/patterns/PatternService.h"
#include "modules/simplify/SimplifyService.h"
#include "modules/visualization/VisualizationService.h"
#include "modules/mbd/MbdService.h"

namespace py = pybind11;
using namespace cad::core;
using namespace cad::modules;
using namespace cad::interop;

PYBIND11_MODULE(cadursor, module) {
    module.doc() = "CADursor Python bindings - Complete CAD application API";
    module.attr("__version__") = "1.0.0";
    
    py::class_<Modeler>(module, "Modeler")
        .def(py::init<>())
        .def("createPart", &Modeler::createPart)
        .def("createAssembly", &Modeler::createAssembly)
        .def("validateSketch", &Modeler::validateSketch)
        .def("evaluateParameters", &Modeler::evaluateParameters)
        .def("solveConstraints", &Modeler::solveConstraints)
        .def("validateConstraints", &Modeler::validateConstraints)
        .def("isOverConstrained", &Modeler::isOverConstrained)
        .def("isUnderConstrained", &Modeler::isUnderConstrained)
        .def("getDegreesOfFreedom", &Modeler::getDegreesOfFreedom)
        .def("applyExtrude", &Modeler::applyExtrude)
        .def("applyRevolve", &Modeler::applyRevolve)
        .def("applyLoft", &Modeler::applyLoft)
        .def("applyHole", &Modeler::applyHole)
        .def("applyFillet", &Modeler::applyFillet);
    
    py::class_<Sketch>(module, "Sketch")
        .def(py::init<const std::string&>())
        .def("name", &Sketch::name)
        .def("addConstraint", &Sketch::addConstraint)
        .def("constraints", &Sketch::constraints)
        .def("addParameter", &Sketch::addParameter)
        .def("parameters", py::overload_cast<>(&Sketch::parameters))
        .def("addLine", &Sketch::addLine)
        .def("addCircle", &Sketch::addCircle)
        .def("addArc", &Sketch::addArc)
        .def("addRectangle", &Sketch::addRectangle)
        .def("addPoint", &Sketch::addPoint)
        .def("geometry", &Sketch::geometry)
        .def("findGeometry", py::overload_cast<const std::string&>(&Sketch::findGeometry, py::const_))
        .def("removeGeometry", &Sketch::removeGeometry);
    
    py::class_<Part>(module, "Part")
        .def(py::init<const std::string&>())
        .def("name", &Part::name)
        .def("addFeature", &Part::addFeature)
        .def("features", &Part::features)
        .def("findFeature", py::overload_cast<const std::string&>(&Part::findFeature, py::const_))
        .def("removeFeature", &Part::removeFeature)
        .def("createExtrude", &Part::createExtrude)
        .def("createRevolve", &Part::createRevolve)
        .def("createLoft", &Part::createLoft)
        .def("createHole", &Part::createHole)
        .def("createFillet", &Part::createFillet)
        .def("createPattern", &Part::createPattern);
    
    py::class_<Assembly>(module, "Assembly")
        .def(py::init<>())
        .def("addComponent", &Assembly::addComponent)
        .def("components", &Assembly::components)
        .def("addMate", &Assembly::addMate)
        .def("mates", &Assembly::mates)
        .def("createMate", &Assembly::createMate)
        .def("createFlush", &Assembly::createFlush)
        .def("createAngle", &Assembly::createAngle)
        .def("createInsert", &Assembly::createInsert)
        .def("solveMates", &Assembly::solveMates)
        .def("findComponent", py::overload_cast<std::uint64_t>(&Assembly::findComponent, py::const_));
    
    py::class_<UndoStack>(module, "UndoStack")
        .def(py::init<>())
        .def("execute", &UndoStack::execute)
        .def("canUndo", &UndoStack::canUndo)
        .def("canRedo", &UndoStack::canRedo)
        .def("undo", &UndoStack::undo)
        .def("redo", &UndoStack::redo)
        .def("clear", &UndoStack::clear)
        .def("getHistorySize", &UndoStack::getHistorySize)
        .def("getUndoCount", &UndoStack::getUndoCount)
        .def("getRedoCount", &UndoStack::getRedoCount)
        .def("getCommandHistory", &UndoStack::getCommandHistory)
        .def("setMaxHistorySize", &UndoStack::setMaxHistorySize);
    
    py::class_<ImportExportService>(module, "ImportExportService")
        .def(py::init<>())
        .def("importModel", &ImportExportService::importModel)
        .def("exportModel", &ImportExportService::exportModel)
        .def("importStep", &ImportExportService::importStep)
        .def("exportStep", &ImportExportService::exportStep)
        .def("importIges", &ImportExportService::importIges)
        .def("exportIges", &ImportExportService::exportIges)
        .def("importStl", &ImportExportService::importStl)
        .def("exportStl", &ImportExportService::exportStl)
        .def("importDwg", &ImportExportService::importDwg)
        .def("importDxf", &ImportExportService::importDxf)
        .def("exportDwg", &ImportExportService::exportDwg)
        .def("exportDxf", &ImportExportService::exportDxf)
        .def("importMultiple", &ImportExportService::importMultiple)
        .def("exportMultiple", &ImportExportService::exportMultiple)
        .def("validateFileFormat", &ImportExportService::validateFileFormat)
        .def("detectFileFormat", &ImportExportService::detectFileFormat);
    
    py::class_<PatternService>(module, "PatternService")
        .def(py::init<>())
        .def("createPattern", &PatternService::createPattern)
        .def("createRectangularPattern", &PatternService::createRectangularPattern)
        .def("createCircularPattern", &PatternService::createCircularPattern)
        .def("createCurvePattern", &PatternService::createCurvePattern)
        .def("editPattern", &PatternService::editPattern)
        .def("suppressInstance", &PatternService::suppressInstance)
        .def("unsuppressInstance", &PatternService::unsuppressInstance)
        .def("deletePattern", &PatternService::deletePattern)
        .def("getPatternInstances", &PatternService::getPatternInstances)
        .def("getPatternType", &PatternService::getPatternType);
    
    py::class_<SimplifyService>(module, "SimplifyService")
        .def(py::init<>())
        .def("simplify", &SimplifyService::simplify)
        .def("simplifyWithRules", &SimplifyService::simplifyWithRules)
        .def("replaceWithBoundingBox", &SimplifyService::replaceWithBoundingBox)
        .def("replaceWithSimplifiedGeometry", &SimplifyService::replaceWithSimplifiedGeometry)
        .def("removeInternalFeatures", &SimplifyService::removeInternalFeatures)
        .def("removeSmallFeatures", &SimplifyService::removeSmallFeatures)
        .def("combineSimilarParts", &SimplifyService::combineSimilarParts)
        .def("getSimplifiedComponents", &SimplifyService::getSimplifiedComponents)
        .def("getSimplificationRatio", &SimplifyService::getSimplificationRatio)
        .def("createPerformancePreset", &SimplifyService::createPerformancePreset)
        .def("createDetailPreset", &SimplifyService::createDetailPreset)
        .def("createBalancedPreset", &SimplifyService::createBalancedPreset);
    
    py::class_<VisualizationService>(module, "VisualizationService")
        .def(py::init<>())
        .def("runVisualization", &VisualizationService::runVisualization)
        .def("createIllustration", &VisualizationService::createIllustration)
        .def("createRendering", &VisualizationService::createRendering)
        .def("createAnimation", &VisualizationService::createAnimation)
        .def("exportToImage", &VisualizationService::exportToImage)
        .def("exportToVideo", &VisualizationService::exportToVideo)
        .def("generatePreview", &VisualizationService::generatePreview);
    
    py::class_<MbdService>(module, "MbdService")
        .def(py::init<>())
        .def("applyMbd", &MbdService::applyMbd)
        .def("buildDefaultPmi", &MbdService::buildDefaultPmi)
        .def("prepareForRendering", &MbdService::prepareForRendering)
        .def("getVisibleAnnotations", &MbdService::getVisibleAnnotations)
        .def("getVisibleDatums", &MbdService::getVisibleDatums)
        .def("getVisibleTolerances", &MbdService::getVisibleTolerances)
        .def("renderMbdInViewport", &MbdService::renderMbdInViewport)
        .def("updateMbdVisibility", &MbdService::updateMbdVisibility)
        .def("getAnnotationsForViewport", &MbdService::getAnnotationsForViewport);
    
    py::enum_<cad::core::ConstraintType>(module, "ConstraintType")
        .value("Coincident", cad::core::ConstraintType::Coincident)
        .value("Horizontal", cad::core::ConstraintType::Horizontal)
        .value("Vertical", cad::core::ConstraintType::Vertical)
        .value("Parallel", cad::core::ConstraintType::Parallel)
        .value("Perpendicular", cad::core::ConstraintType::Perpendicular)
        .value("Tangent", cad::core::ConstraintType::Tangent)
        .value("Equal", cad::core::ConstraintType::Equal)
        .value("Distance", cad::core::ConstraintType::Distance)
        .value("Angle", cad::core::ConstraintType::Angle);
    
    py::enum_<cad::core::GeometryType>(module, "GeometryType")
        .value("Point", cad::core::GeometryType::Point)
        .value("Line", cad::core::GeometryType::Line)
        .value("Circle", cad::core::GeometryType::Circle)
        .value("Arc", cad::core::GeometryType::Arc)
        .value("Rectangle", cad::core::GeometryType::Rectangle);
    
    py::enum_<cad::core::FeatureType>(module, "FeatureType")
        .value("Extrude", cad::core::FeatureType::Extrude)
        .value("Revolve", cad::core::FeatureType::Revolve)
        .value("Loft", cad::core::FeatureType::Loft)
        .value("Hole", cad::core::FeatureType::Hole)
        .value("Fillet", cad::core::FeatureType::Fillet)
        .value("Chamfer", cad::core::FeatureType::Chamfer)
        .value("Shell", cad::core::FeatureType::Shell)
        .value("Pattern", cad::core::FeatureType::Pattern);
    
    py::enum_<cad::core::MateType>(module, "MateType")
        .value("Mate", cad::core::MateType::Mate)
        .value("Flush", cad::core::MateType::Flush)
        .value("Angle", cad::core::MateType::Angle)
        .value("Insert", cad::core::MateType::Insert);
    
    module.def("create_modeler", []() { return std::make_unique<Modeler>(); });
    module.def("create_undo_stack", []() { return std::make_unique<UndoStack>(); });
    module.def("create_import_export_service", []() { return std::make_unique<ImportExportService>(); });
}
