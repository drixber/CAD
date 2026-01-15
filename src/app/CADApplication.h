#pragma once

namespace cad {
namespace app {

class CADApplication {
public:
    CADApplication();
    ~CADApplication();

    bool initialize();
    void shutdown();

    void setController(class AppController* controller);

private:
    AppController* controller_{nullptr};
};

}  // namespace app
}  // namespace cad
