#include "Viewport3D.h"

#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QResizeEvent>
#include <QWheelEvent>
#include <QColor>
#include <cmath>

namespace cad {
namespace ui {

Viewport3D::Viewport3D(QWidget* parent) : QWidget(parent) {
    setMinimumSize(400, 300);
    setFocusPolicy(Qt::StrongFocus);
    initializeViewport();
}

Viewport3D::~Viewport3D() {
    // Cleanup Coin3D/OCCT resources
    if (coin3d_viewer_) {
        // delete coin3d_viewer_;
    }
    if (occt_viewer_) {
        // delete occt_viewer_;
    }
}

void Viewport3D::initializeViewport() {
    // In real implementation: initialize Coin3D or OpenCascade viewer
    // coin3d_viewer_ = new SoQtExaminerViewer(this);
    // scene_graph_root_ = new SoSeparator();
    // coin3d_viewer_->setSceneGraph(static_cast<SoSeparator*>(scene_graph_root_));
    // or
    // occt_viewer_ = new V3d_View(...);
    
    resetCamera();
}

void Viewport3D::renderGeometry(const std::string& geometry_id, void* geometry_handle) {
    // In real implementation: render geometry in viewport
    // void* coin3d_node = createCoin3DNode(geometry_id, geometry_handle);
    // if (coin3d_node) {
    //     geometry_nodes_[geometry_id] = coin3d_node;
    //     addNodeToSceneGraph(coin3d_node);
    //     updateSceneGraphDisplayMode();
    // }
    
    // Track rendered geometry
    if (std::find(rendered_geometry_ids_.begin(), rendered_geometry_ids_.end(), geometry_id) == rendered_geometry_ids_.end()) {
        rendered_geometry_ids_.push_back(geometry_id);
    }
    if (geometry_handle) {
        geometry_nodes_[geometry_id] = geometry_handle;
    }
    updateView();
}

void Viewport3D::renderAssembly(const std::string& assembly_id) {
    // In real implementation: render assembly in viewport
    updateView();
}

void Viewport3D::renderMbdAnnotations(const std::vector<void*>& annotation_handles) {
    // In real implementation: render MBD annotations in 3D scene
    // for (void* handle : annotation_handles) {
    //     SoAnnotation* annotation = static_cast<SoAnnotation*>(handle);
    //     // Add to scene graph
    // }
    updateView();
}

void Viewport3D::clearScene() {
    // In real implementation: clear viewport scene
    // if (scene_graph_root_) {
    //     SoSeparator* root = static_cast<SoSeparator*>(scene_graph_root_);
    //     root->removeAllChildren();
    // }
    rendered_geometry_ids_.clear();
    geometry_nodes_.clear();
    assembly_nodes_.clear();
    annotation_nodes_.clear();
    updateView();
}

void Viewport3D::updateView() {
    update();  // Trigger repaint
    emit viewportUpdated();
}

void Viewport3D::setCamera(const ViewportCamera& camera) {
    camera_ = camera;
    // In real implementation: update viewport camera
    // coin3d_viewer_->setCameraPosition(...);
    updateView();
}

ViewportCamera Viewport3D::getCamera() const {
    return camera_;
}

void Viewport3D::resetCamera() {
    camera_.position_x = 0.0;
    camera_.position_y = 0.0;
    camera_.position_z = 10.0;
    camera_.target_x = 0.0;
    camera_.target_y = 0.0;
    camera_.target_z = 0.0;
    camera_.up_x = 0.0;
    camera_.up_y = 1.0;
    camera_.up_z = 0.0;
    camera_.field_of_view = 45.0;
    setCamera(camera_);
}

void Viewport3D::fitToView() {
    // In real implementation: fit all objects to view
    // coin3d_viewer_->viewAll();
    updateView();
}

void Viewport3D::setSettings(const ViewportSettings& settings) {
    settings_ = settings;
    // In real implementation: apply settings to viewport
    updateView();
}

ViewportSettings Viewport3D::getSettings() const {
    return settings_;
}

void Viewport3D::enableSelection(bool enabled) {
    selection_enabled_ = enabled;
}

std::vector<std::string> Viewport3D::getSelectedObjects() const {
    return selected_objects_;
}

void Viewport3D::selectObject(const std::string& object_id) {
    if (std::find(selected_objects_.begin(), selected_objects_.end(), object_id) == selected_objects_.end()) {
        selected_objects_.push_back(object_id);
        emit objectSelected(object_id);
        updateView();
    }
}

void Viewport3D::clearSelection() {
    selected_objects_.clear();
    updateView();
}

void Viewport3D::highlightObject(const std::string& object_id, bool highlight) {
    if (highlight) {
        if (std::find(highlighted_objects_.begin(), highlighted_objects_.end(), object_id) == highlighted_objects_.end()) {
            highlighted_objects_.push_back(object_id);
        }
    } else {
        highlighted_objects_.erase(
            std::remove(highlighted_objects_.begin(), highlighted_objects_.end(), object_id),
            highlighted_objects_.end()
        );
    }
    updateView();
}

void Viewport3D::setDisplayMode(DisplayMode mode) {
    display_mode_ = mode;
    // In real implementation: update Coin3D display mode
    // updateSceneGraphDisplayMode();
    updateView();
}

Viewport3D::DisplayMode Viewport3D::getDisplayMode() const {
    return display_mode_;
}

void Viewport3D::paintEvent(QPaintEvent* event) {
    QWidget::paintEvent(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Fill background
    QColor bg_color(settings_.background_color.c_str());
    if (!bg_color.isValid()) {
        bg_color = QColor(43, 43, 43);  // Default dark gray
    }
    painter.fillRect(rect(), bg_color);
    
    if (settings_.show_grid) {
        renderGrid(painter);
    }
    
    if (settings_.show_axes) {
        renderAxes(painter);
    }
    
    // Render any geometry that has been added
    renderScene(painter);
}

void Viewport3D::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    // In real implementation: update viewport size
    // coin3d_viewer_->setSize(width(), height());
    updateView();
}

void Viewport3D::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        is_dragging_ = true;
        last_mouse_x_ = event->x();
        last_mouse_y_ = event->y();
        drag_mode_ = "orbit";  // Default to orbit
        
        if (selection_enabled_) {
            // In real implementation: pick object at mouse position using ray-casting
            std::string object_id = pickObjectAt(event->x(), event->y());
            if (!object_id.empty()) {
                selectObject(object_id);
            }
        }
    } else if (event->button() == Qt::MiddleButton) {
        is_dragging_ = true;
        last_mouse_x_ = event->x();
        last_mouse_y_ = event->y();
        drag_mode_ = "pan";
    }
    QWidget::mousePressEvent(event);
}

void Viewport3D::mouseMoveEvent(QMouseEvent* event) {
    if (is_dragging_) {
        int dx = event->x() - last_mouse_x_;
        int dy = event->y() - last_mouse_y_;
        
        if (drag_mode_ == "orbit") {
            // Rotate camera around target
            double angle_x = dx * 0.01;
            double angle_y = dy * 0.01;
            
            // Simple rotation around Y axis (horizontal)
            double radius = std::sqrt(
                (camera_.position_x - camera_.target_x) * (camera_.position_x - camera_.target_x) +
                (camera_.position_y - camera_.target_y) * (camera_.position_y - camera_.target_y) +
                (camera_.position_z - camera_.target_z) * (camera_.position_z - camera_.target_z)
            );
            
            static double current_angle = 0.0;
            current_angle += angle_x;
            
            camera_.position_x = camera_.target_x + radius * std::sin(current_angle);
            camera_.position_z = camera_.target_z + radius * std::cos(current_angle);
            camera_.position_y = camera_.target_y + angle_y * radius;
            
            setCamera(camera_);
        } else if (drag_mode_ == "pan") {
            // Pan camera
            double pan_speed = 0.01;
            camera_.target_x -= dx * pan_speed;
            camera_.target_y += dy * pan_speed;
            camera_.position_x -= dx * pan_speed;
            camera_.position_y += dy * pan_speed;
            setCamera(camera_);
        }
        
        last_mouse_x_ = event->x();
        last_mouse_y_ = event->y();
    }
    QWidget::mouseMoveEvent(event);
}

void Viewport3D::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton || event->button() == Qt::MiddleButton) {
        is_dragging_ = false;
    }
    QWidget::mouseReleaseEvent(event);
}

void Viewport3D::wheelEvent(QWheelEvent* event) {
    // Handle zoom
    double zoom_factor = 1.0 + (event->angleDelta().y() / 1200.0);
    double current_distance = std::sqrt(
        (camera_.position_x - camera_.target_x) * (camera_.position_x - camera_.target_x) +
        (camera_.position_y - camera_.target_y) * (camera_.position_y - camera_.target_y) +
        (camera_.position_z - camera_.target_z) * (camera_.position_z - camera_.target_z)
    );
    
    double new_distance = current_distance / zoom_factor;
    double dx = camera_.position_x - camera_.target_x;
    double dy = camera_.position_y - camera_.target_y;
    double dz = camera_.position_z - camera_.target_z;
    double length = std::sqrt(dx*dx + dy*dy + dz*dz);
    
    if (length > 0.001) {
        camera_.position_x = camera_.target_x + (dx / length) * new_distance;
        camera_.position_y = camera_.target_y + (dy / length) * new_distance;
        camera_.position_z = camera_.target_z + (dz / length) * new_distance;
        setCamera(camera_);
    }
    
    QWidget::wheelEvent(event);
}

void Viewport3D::renderGrid(QPainter& painter) {
    painter.setPen(QPen(QColor(100, 100, 100, 100), 1));
    
    int center_x = width() / 2;
    int center_y = height() / 2;
    double grid_size = settings_.grid_size * 10;  // Scale for display
    
    // Draw grid lines
    for (int i = -20; i <= 20; ++i) {
        int x = center_x + static_cast<int>(i * grid_size);
        int y = center_y + static_cast<int>(i * grid_size);
        
        // Vertical lines
        if (x >= 0 && x < width()) {
            painter.drawLine(x, 0, x, height());
        }
        
        // Horizontal lines
        if (y >= 0 && y < height()) {
            painter.drawLine(0, y, width(), y);
        }
    }
    
    // Draw center lines
    painter.setPen(QPen(QColor(150, 150, 150, 150), 2));
    painter.drawLine(center_x, 0, center_x, height());
    painter.drawLine(0, center_y, width(), center_y);
}

void Viewport3D::renderAxes(QPainter& painter) {
    int center_x = width() / 2;
    int center_y = height() / 2;
    int axis_length = 50;
    
    // X axis (red)
    painter.setPen(QPen(QColor(255, 0, 0), 2));
    painter.drawLine(center_x, center_y, center_x + axis_length, center_y);
    painter.drawText(center_x + axis_length + 5, center_y - 5, "X");
    
    // Y axis (green)
    painter.setPen(QPen(QColor(0, 255, 0), 2));
    painter.drawLine(center_x, center_y, center_x, center_y - axis_length);
    painter.drawText(center_x + 5, center_y - axis_length - 5, "Y");
    
    // Z axis (blue) - represented as diagonal
    painter.setPen(QPen(QColor(0, 0, 255), 2));
    int z_x = center_x + static_cast<int>(axis_length * 0.707);
    int z_y = center_y - static_cast<int>(axis_length * 0.707);
    painter.drawLine(center_x, center_y, z_x, z_y);
    painter.drawText(z_x + 5, z_y - 5, "Z");
}

void Viewport3D::renderScene(QPainter& painter) {
    // Render any stored geometry
    // In real implementation: would render 3D geometry using proper projection
    // For now, just show that rendering is active
    if (!rendered_geometry_ids_.empty()) {
        QString display_mode_str;
        switch (display_mode_) {
            case DisplayMode::Wireframe:
                display_mode_str = "Wireframe";
                break;
            case DisplayMode::Shaded:
                display_mode_str = "Shaded";
                break;
            case DisplayMode::HiddenLine:
                display_mode_str = "HiddenLine";
                break;
        }
        
        painter.setPen(QPen(QColor(200, 200, 200), 2));
        painter.drawText(10, 20, QString("Rendering %1 object(s) - %2").arg(rendered_geometry_ids_.size()).arg(display_mode_str));
        
        // Show selection info
        if (!selected_objects_.empty()) {
            painter.setPen(QPen(QColor(255, 255, 0), 2));
            painter.drawText(10, 40, QString("Selected: %1 object(s)").arg(selected_objects_.size()));
        }
        
        // Show highlighted objects
        if (!highlighted_objects_.empty()) {
            painter.setPen(QPen(QColor(0, 255, 255), 2));
            painter.drawText(10, 60, QString("Highlighted: %1 object(s)").arg(highlighted_objects_.size()));
        }
    }
}

void* Viewport3D::createCoin3DNode(const std::string& geometry_id, void* geometry_handle) const {
    // In real implementation: create Coin3D node from geometry handle
    // SoSeparator* node = new SoSeparator();
    // // Convert geometry_handle to Coin3D geometry
    // // Add material, transform, etc.
    // return node;
    (void)geometry_id;
    (void)geometry_handle;
    return nullptr;
}

void Viewport3D::addNodeToSceneGraph(void* node) {
    // In real implementation: add node to Coin3D scene graph
    // if (scene_graph_root_ && node) {
    //     SoSeparator* root = static_cast<SoSeparator*>(scene_graph_root_);
    //     root->addChild(static_cast<SoNode*>(node));
    // }
    (void)node;
}

void Viewport3D::removeNodeFromSceneGraph(const std::string& geometry_id) {
    // In real implementation: remove node from Coin3D scene graph
    // auto it = geometry_nodes_.find(geometry_id);
    // if (it != geometry_nodes_.end() && scene_graph_root_) {
    //     SoSeparator* root = static_cast<SoSeparator*>(scene_graph_root_);
    //     root->removeChild(static_cast<SoNode*>(it->second));
    //     geometry_nodes_.erase(it);
    // }
    (void)geometry_id;
}

std::string Viewport3D::pickObjectAt(int x, int y) const {
    // In real implementation: ray-casting to pick object at screen coordinates
    // if (coin3d_viewer_) {
    //     SoRayPickAction pick_action(coin3d_viewer_->getViewportRegion());
    //     pick_action.setPoint(SbVec2s(x, y));
    //     pick_action.apply(static_cast<SoNode*>(scene_graph_root_));
    //     SoPickedPoint* picked = pick_action.getPickedPoint();
    //     if (picked) {
    //         // Extract object ID from picked point
    //         return extractObjectId(picked);
    //     }
    // }
    (void)x;
    (void)y;
    return {};
}

void Viewport3D::updateSceneGraphDisplayMode() {
    // In real implementation: update Coin3D display mode for all nodes
    // SoDrawStyle* draw_style = new SoDrawStyle();
    // switch (display_mode_) {
    //     case DisplayMode::Wireframe:
    //         draw_style->style = SoDrawStyle::LINES;
    //         break;
    //     case DisplayMode::Shaded:
    //         draw_style->style = SoDrawStyle::FILLED;
    //         break;
    //     case DisplayMode::HiddenLine:
    //         draw_style->style = SoDrawStyle::FILLED;
    //         // Add hidden line removal
    //         break;
    // }
    // // Apply to scene graph
}

}  // namespace ui
}  // namespace cad

