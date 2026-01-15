#include "CADApplication.h"

#include "AppController.h"

namespace cad {
namespace app {

CADApplication::CADApplication() = default;
CADApplication::~CADApplication() = default;

bool CADApplication::initialize() {
    if (controller_) {
        controller_->initialize();
    }
    return true;
}

void CADApplication::shutdown() {}

void CADApplication::setController(AppController* controller) {
    controller_ = controller;
}

}  // namespace app
}  // namespace cad
