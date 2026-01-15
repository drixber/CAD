#include "FreeCADAdapter.h"

#ifdef CAD_USE_FREECAD
#include <App/Application.h>
#include <App/Document.h>
#include <App/DocumentObject.h>
#endif

namespace cad {
namespace core {

FreeCADAdapter::FreeCADAdapter() = default;

bool FreeCADAdapter::initializeSession() {
#ifdef CAD_USE_FREECAD
    initialized_ = (App::GetApplication() != nullptr);
    return initialized_;
#else
    initialized_ = false;
    return false;
#endif
}

bool FreeCADAdapter::createDocument(const std::string& name) {
    if (!initialized_) {
        return false;
    }
#ifdef CAD_USE_FREECAD
    App::Application* app = App::GetApplication();
    if (!app) {
        return false;
    }
    App::Document* doc = app->newDocument(name.c_str(), name.c_str(), true, false);
    if (!doc) {
        return false;
    }
    active_document_ = doc->getName();
    return true;
#else
    (void)name;
    return false;
#endif
}

const std::string& FreeCADAdapter::activeDocument() const {
    return active_document_;
}

bool FreeCADAdapter::createSketchStub(const std::string& name) const {
#ifdef CAD_USE_FREECAD
    App::Application* app = App::GetApplication();
    if (!app) {
        return false;
    }
    App::Document* doc = app->getActiveDocument();
    if (!doc) {
        return false;
    }
    App::DocumentObject* obj = doc->addObject("Sketcher::SketchObject", name.c_str());
    if (!obj) {
        return false;
    }
    doc->recompute();
    return true;
#else
    (void)name;
    return false;
#endif
}

bool FreeCADAdapter::syncSketch(const Sketch& sketch) const {
#ifdef CAD_USE_FREECAD
    if (sketch.name().empty()) {
        return false;
    }
    if (!createSketchStub(sketch.name())) {
        return false;
    }
    return true;
#else
    (void)sketch;
    return false;
#endif
}

bool FreeCADAdapter::syncConstraints(const Sketch& sketch) const {
#ifdef CAD_USE_FREECAD
    if (sketch.constraints().empty()) {
        return true;
    }
    // Placeholder: actual FreeCAD constraint mapping will be implemented later.
    return true;
#else
    (void)sketch;
    return false;
#endif
}

bool FreeCADAdapter::createDrawingStub(const std::string& name) const {
#ifdef CAD_USE_FREECAD
    App::Application* app = App::GetApplication();
    if (!app) {
        return false;
    }
    App::Document* doc = app->getActiveDocument();
    if (!doc) {
        return false;
    }
    App::DocumentObject* obj = doc->addObject("TechDraw::DrawPage", name.c_str());
    if (!obj) {
        return false;
    }
    doc->recompute();
    return true;
#else
    (void)name;
    return false;
#endif
}

Part FreeCADAdapter::buildPartFromSketch(const Sketch& sketch) const {
#ifdef CAD_USE_FREECAD
    (void)sketch;
    App::Application* app = App::GetApplication();
    if (!app) {
        return Part("FreeCADPart");
    }
    App::Document* doc = app->getActiveDocument();
    if (!doc) {
        return Part("FreeCADPart");
    }
    App::DocumentObject* obj = doc->addObject("Part::Box", "Box");
    if (obj) {
        doc->recompute();
        return Part(obj->getNameInDocument());
    }
    return Part("FreeCADPart");
#else
    Modeler modeler;
    return modeler.createPart(sketch);
#endif
}

bool FreeCADAdapter::isAvailable() const {
    #ifdef CAD_USE_FREECAD
    return true;
    #else
    return false;
    #endif
}

}  // namespace core
}  // namespace cad
