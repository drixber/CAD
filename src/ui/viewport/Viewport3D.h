#pragma once

#include <QWidget>
#include <string>
#include <vector>
#include <memory>
#include "RenderEngine3D.h"
#include "SoQtViewerIntegration.h"

namespace cad {
namespace ui {

struct ViewportCamera {
    double position_x{0.0};
    double position_y{0.0};
    double position_z{10.0};
    double target_x{0.0};
    double target_y{0.0};
    double target_z{0.0};
    double up_x{0.0};
    double up_y{1.0};
    double up_z{0.0};
    double field_of_view{45.0};
};

struct ViewportSettings {
    bool show_grid{true};
    bool show_axes{true};
    bool show_background{true};
    std::string background_color{"#2b2b2b"};
    double grid_size{10.0};
    int grid_subdivisions{10};
};

class Viewport3D : public QWidget {
    Q_OBJECT

public:
    explicit Viewport3D(QWidget* parent = nullptr);
    ~Viewport3D();
    
    // Rendering
    void renderGeometry(const std::string& geometry_id, void* geometry_handle);
    void renderAssembly(const std::string& assembly_id);
    void renderMbdAnnotations(const std::vector<void*>& annotation_handles);
    void clearScene();
    void updateView();
    
    // Camera control
    void setCamera(const ViewportCamera& camera);
    ViewportCamera getCamera() const;
    void resetCamera();
    void fitToView();
    
    // Viewport settings
    void setSettings(const ViewportSettings& settings);
    ViewportSettings getSettings() const;
    
    // Interaction
    void enableSelection(bool enabled);
    std::vector<std::string> getSelectedObjects() const;
    void selectObject(const std::string& object_id);
    void clearSelection();
    void highlightObject(const std::string& object_id, bool highlight = true);
    
    // Viewport display modes
    enum class DisplayMode {
        Wireframe,
        Shaded,
        HiddenLine
    };
    void setDisplayMode(DisplayMode mode);
    DisplayMode getDisplayMode() const;
    
signals:
    void objectSelected(const std::string& object_id);
    void viewportUpdated();

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    std::unique_ptr<RenderEngine3D> render_engine_;
    std::unique_ptr<SoQtViewerIntegration> soqt_viewer_;
    ViewportCamera camera_;
    ViewportSettings settings_;
    bool selection_enabled_{true};
    std::vector<std::string> selected_objects_;
    std::vector<std::string> rendered_geometry_ids_;
    std::vector<std::string> highlighted_objects_;
    DisplayMode display_mode_{DisplayMode::Shaded};
    
    bool is_dragging_{false};
    int last_mouse_x_{0};
    int last_mouse_y_{0};
    std::string drag_mode_{"orbit"};
    
    std::map<std::string, GeometryData> geometry_data_;
    std::map<std::string, std::vector<std::string>> assembly_components_;
    std::map<std::string, std::vector<void*>> annotation_data_;
    
    void initializeViewport();
    void renderGrid(QPainter& painter);
    void renderAxes(QPainter& painter);
    void renderScene(QPainter& painter);
    
    GeometryData createGeometryData(const std::string& geometry_id, void* geometry_handle) const;
    void addGeometryToScene(const std::string& geometry_id, const GeometryData& data);
    void removeGeometryFromScene(const std::string& geometry_id);
    std::string pickObjectAt(int x, int y) const;
    void updateDisplayMode();
    void convertGeometryHandleToData(void* handle, GeometryData& data) const;
    bool useSoQtViewer() const;
};

}  // namespace ui
}  // namespace cad

