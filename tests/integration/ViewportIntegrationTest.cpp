#include <gtest/gtest.h>
#include "ui/viewport/Viewport3D.h"
#include <QApplication>

using namespace cad::ui;

class ViewportIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        int argc = 0;
        char** argv = nullptr;
        app_ = std::make_unique<QApplication>(argc, argv);
    }
    
    void TearDown() override {
        app_.reset();
    }
    
    std::unique_ptr<QApplication> app_;
};

TEST_F(ViewportIntegrationTest, ViewportInitialization) {
    Viewport3D viewport;
    
    ViewportCamera camera;
    camera.position_z = 10.0;
    viewport.setCamera(camera);
    
    ViewportCamera retrieved = viewport.getCamera();
    ASSERT_DOUBLE_EQ(retrieved.position_z, 10.0);
}

TEST_F(ViewportIntegrationTest, ViewportSettings) {
    Viewport3D viewport;
    
    ViewportSettings settings;
    settings.show_grid = true;
    settings.show_axes = true;
    viewport.setSettings(settings);
    
    ViewportSettings retrieved = viewport.getSettings();
    ASSERT_TRUE(retrieved.show_grid);
    ASSERT_TRUE(retrieved.show_axes);
}

TEST_F(ViewportIntegrationTest, DisplayModes) {
    Viewport3D viewport;
    
    viewport.setDisplayMode(Viewport3D::DisplayMode::Wireframe);
    ASSERT_EQ(viewport.getDisplayMode(), Viewport3D::DisplayMode::Wireframe);
    
    viewport.setDisplayMode(Viewport3D::DisplayMode::Shaded);
    ASSERT_EQ(viewport.getDisplayMode(), Viewport3D::DisplayMode::Shaded);
}
