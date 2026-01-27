#include "Viewport3D.h"
#include "RenderEngine3D.h"
#include "SoQtViewerIntegration.h"

#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QResizeEvent>
#include <QWheelEvent>
#include <QColor>
#include <QPolygon>
#include <cmath>
#include <map>
#include <algorithm>
#include <limits>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace cad {
namespace ui {

Viewport3D::Viewport3D(::QWidget* parent) : ::QWidget(parent) {
    setMinimumSize(400, 300);
    setFocusPolicy(Qt::StrongFocus);
    render_engine_ = std::make_unique<RenderEngine3D>();
#ifdef CAD_USE_COIN3D
#ifdef CAD_USE_QT
    soqt_viewer_ = std::make_unique<SoQtViewerIntegration>();
    if (soqt_viewer_->initialize(this)) {
        ::QWidget* viewer_widget = soqt_viewer_->getWidget();
        if (viewer_widget) {
            viewer_widget->setParent(this);
            viewer_widget->setGeometry(0, 0, width(), height());
            viewer_widget->show();
        }
    }
#endif
#endif
    initializeViewport();
}

Viewport3D::~Viewport3D() {
    if (render_engine_) {
        render_engine_->shutdown();
    }
}

void Viewport3D::initializeViewport() {
    if (useSoQtViewer()) {
        if (soqt_viewer_) {
            soqt_viewer_->setCamera(
                camera_.position_x, camera_.position_y, camera_.position_z,
                camera_.target_x, camera_.target_y, camera_.target_z,
                camera_.up_x, camera_.up_y, camera_.up_z,
                camera_.field_of_view
            );
            QColor bg_color(settings_.background_color.c_str());
            if (bg_color.isValid()) {
                soqt_viewer_->setBackgroundColor(
                    bg_color.redF(), bg_color.greenF(), bg_color.blueF()
                );
            }
            soqt_viewer_->showGrid(settings_.show_grid);
            soqt_viewer_->showAxes(settings_.show_axes);
        }
    } else {
        if (render_engine_) {
            render_engine_->initialize(width(), height());
        }
    }
    resetCamera();
}

void Viewport3D::renderGeometry(const std::string& geometry_id, void* geometry_handle) {
    if (useSoQtViewer()) {
        if (soqt_viewer_ && geometry_handle) {
            soqt_viewer_->addGeometryNode(geometry_id, geometry_handle);
            if (std::find(rendered_geometry_ids_.begin(), rendered_geometry_ids_.end(), geometry_id) == rendered_geometry_ids_.end()) {
                rendered_geometry_ids_.push_back(geometry_id);
            }
        }
    } else {
        if (!render_engine_ || !render_engine_->isInitialized()) {
            return;
        }
        
        GeometryData data = createGeometryData(geometry_id, geometry_handle);
        convertGeometryHandleToData(geometry_handle, data);
        
        std::string created_id = render_engine_->createGeometry(data);
        if (!created_id.empty()) {
            geometry_data_[geometry_id] = data;
            addGeometryToScene(geometry_id, data);
            
            if (std::find(rendered_geometry_ids_.begin(), rendered_geometry_ids_.end(), geometry_id) == rendered_geometry_ids_.end()) {
                rendered_geometry_ids_.push_back(geometry_id);
            }
            
            updateDisplayMode();
        }
    }
    
    updateView();
}

void Viewport3D::renderAssembly(const std::string& assembly_id) {
    if (useSoQtViewer()) {
        if (soqt_viewer_) {
            if (assembly_components_.find(assembly_id) == assembly_components_.end()) {
                assembly_components_[assembly_id] = {};
            }
            
            for (const auto& component_id : assembly_components_[assembly_id]) {
                auto it = geometry_data_.find(component_id);
                if (it != geometry_data_.end() && it->second.native_handle) {
                    soqt_viewer_->addGeometryNode(component_id, it->second.native_handle);
                    if (std::find(rendered_geometry_ids_.begin(), rendered_geometry_ids_.end(), component_id) == rendered_geometry_ids_.end()) {
                        rendered_geometry_ids_.push_back(component_id);
                    }
                }
            }
        }
    } else {
        if (!render_engine_ || !render_engine_->isInitialized()) {
            return;
        }
        
        if (assembly_components_.find(assembly_id) == assembly_components_.end()) {
            assembly_components_[assembly_id] = {};
        }
        
        for (const auto& component_id : assembly_components_[assembly_id]) {
            auto it = geometry_data_.find(component_id);
            if (it != geometry_data_.end()) {
                render_engine_->addToScene(component_id);
            }
        }
    }
    
    updateView();
}

void Viewport3D::renderMbdAnnotations(const std::vector<void*>& annotation_handles) {
    if (useSoQtViewer()) {
        if (soqt_viewer_) {
            for (size_t i = 0; i < annotation_handles.size(); ++i) {
                std::string annotation_id = "annotation_" + std::to_string(i);
                void* handle = annotation_handles[i];
                
                if (handle) {
                    soqt_viewer_->addGeometryNode(annotation_id, handle);
                    if (std::find(rendered_geometry_ids_.begin(), rendered_geometry_ids_.end(), annotation_id) == rendered_geometry_ids_.end()) {
                        rendered_geometry_ids_.push_back(annotation_id);
                    }
                }
                
                annotation_data_[annotation_id].push_back(handle);
            }
        }
    } else {
        if (!render_engine_ || !render_engine_->isInitialized()) {
            return;
        }
        
        for (size_t i = 0; i < annotation_handles.size(); ++i) {
            std::string annotation_id = "annotation_" + std::to_string(i);
            void* handle = annotation_handles[i];
            
            GeometryData data;
            data.id = annotation_id;
            data.type = GeometryData::Custom;
            data.params[0] = 1.0;
            data.params[1] = 1.0;
            data.params[2] = 0.0;
            data.params[3] = 1.0;
            data.native_handle = handle;
            
            render_engine_->createGeometry(data);
            render_engine_->addToScene(annotation_id);
            
            annotation_data_[annotation_id].push_back(handle);
        }
    }
    
    updateView();
}

void Viewport3D::clearScene() {
    if (useSoQtViewer()) {
        if (soqt_viewer_) {
            for (const auto& geom_id : rendered_geometry_ids_) {
                soqt_viewer_->removeGeometryNode(geom_id);
            }
        }
    } else {
        if (render_engine_ && render_engine_->isInitialized()) {
            render_engine_->clearScene();
        }
    }
    
    rendered_geometry_ids_.clear();
    geometry_data_.clear();
    assembly_components_.clear();
    annotation_data_.clear();
    selected_objects_.clear();
    highlighted_objects_.clear();
    
    updateView();
}

void Viewport3D::updateView() {
    update();  // Trigger repaint
    emit viewportUpdated();
}

void Viewport3D::setCamera(const ViewportCamera& camera) {
    camera_ = camera;
    
    if (useSoQtViewer()) {
        if (soqt_viewer_) {
            soqt_viewer_->setCamera(
                camera.position_x, camera.position_y, camera.position_z,
                camera.target_x, camera.target_y, camera.target_z,
                camera.up_x, camera.up_y, camera.up_z,
                camera.field_of_view
            );
        }
    } else {
        if (render_engine_ && render_engine_->isInitialized()) {
            render_engine_->setCamera(
                camera.position_x, camera.position_y, camera.position_z,
                camera.target_x, camera.target_y, camera.target_z,
                camera.up_x, camera.up_y, camera.up_z,
                camera.field_of_view
            );
        }
    }
    
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
    
    if (useSoQtViewer()) {
        if (soqt_viewer_) {
            soqt_viewer_->resetCamera();
        }
    }
    
    setCamera(camera_);
}

void Viewport3D::fitToView() {
    if (useSoQtViewer()) {
        if (soqt_viewer_) {
            soqt_viewer_->fitToView();
        }
    } else {
        if (!render_engine_ || rendered_geometry_ids_.empty()) {
            return;
        }
        
        double min_x = std::numeric_limits<double>::max();
        double max_x = std::numeric_limits<double>::lowest();
        double min_y = std::numeric_limits<double>::max();
        double max_y = std::numeric_limits<double>::lowest();
        double min_z = std::numeric_limits<double>::max();
        double max_z = std::numeric_limits<double>::lowest();
    
        for (const auto& geom_id : rendered_geometry_ids_) {
            auto it = geometry_data_.find(geom_id);
            if (it != geometry_data_.end()) {
                const GeometryData& data = it->second;
                min_x = std::min(min_x, data.params[4] - 1.0);
                max_x = std::max(max_x, data.params[4] + 1.0);
                min_y = std::min(min_y, data.params[5] - 1.0);
                max_y = std::max(max_y, data.params[5] + 1.0);
                min_z = std::min(min_z, data.params[6] - 1.0);
                max_z = std::max(max_z, data.params[6] + 1.0);
            }
        }
    
    double center_x = (min_x + max_x) * 0.5;
    double center_y = (min_y + max_y) * 0.5;
    double center_z = (min_z + max_z) * 0.5;
    
    double size = std::max({max_x - min_x, max_y - min_y, max_z - min_z});
    double distance = size * 2.0;
    
    camera_.target_x = center_x;
    camera_.target_y = center_y;
    camera_.target_z = center_z;
    camera_.position_x = center_x;
    camera_.position_y = center_y;
    camera_.position_z = center_z + distance;
    
        setCamera(camera_);
        updateView();
    }
}

void Viewport3D::setSettings(const ViewportSettings& settings) {
    settings_ = settings;
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
    
    if (useSoQtViewer()) {
        if (soqt_viewer_) {
            int mode_int = static_cast<int>(mode);
            soqt_viewer_->setDisplayMode(mode_int);
        }
    } else {
        updateDisplayMode();
    }
    
    updateView();
}

Viewport3D::DisplayMode Viewport3D::getDisplayMode() const {
    return display_mode_;
}

void Viewport3D::paintEvent(QPaintEvent* event) {
    if (useSoQtViewer()) {
        QWidget::paintEvent(event);
        return;
    }
    
    QWidget::paintEvent(event);
    
    if (render_engine_ && render_engine_->isInitialized()) {
        render_engine_->render();
    }
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    QColor bg_color(settings_.background_color.c_str());
    if (!bg_color.isValid()) {
        bg_color = QColor(43, 43, 43);
    }
    painter.fillRect(rect(), bg_color);
    
    if (settings_.show_grid) {
        renderGrid(painter);
    }
    
    if (settings_.show_axes) {
        renderAxes(painter);
    }
    
    renderScene(painter);
}

void Viewport3D::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    
    if (useSoQtViewer()) {
        if (soqt_viewer_) {
            ::QWidget* viewer_widget = static_cast<::QWidget*>(soqt_viewer_->getWidget());
            if (viewer_widget) {
                viewer_widget->setGeometry(0, 0, width(), height());
            }
        }
    } else {
        if (render_engine_ && render_engine_->isInitialized()) {
            render_engine_->setViewportSize(width(), height());
        }
    }
    
    updateView();
}

void Viewport3D::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        is_dragging_ = true;
        last_mouse_x_ = event->x();
        last_mouse_y_ = event->y();
        drag_mode_ = "orbit";  // Default to orbit
        
        if (selection_enabled_) {
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
    // Render 3D geometry using isometric projection
    if (rendered_geometry_ids_.empty()) {
        return;
    }
    
    int center_x = width() / 2;
    int center_y = height() / 2;
    double scale = 20.0;  // Scale factor for 3D to 2D projection
    
    // Isometric projection matrix
    // Standard isometric angles: 30 degrees
    const double iso_angle = 30.0 * M_PI / 180.0;
    const double cos_iso = std::cos(iso_angle);
    const double sin_iso = std::sin(iso_angle);
    
    // Project 3D point to 2D screen coordinates
    auto project3D = [&](double x, double y, double z) -> QPoint {
        // Apply camera transform
        double dx = x - camera_.target_x;
        double dy = y - camera_.target_y;
        double dz = z - camera_.target_z;
        
        // Simple rotation around Y axis (based on camera position)
        double cam_angle = std::atan2(camera_.position_x - camera_.target_x, 
                                      camera_.position_z - camera_.target_z);
        double cos_cam = std::cos(cam_angle);
        double sin_cam = std::sin(cam_angle);
        double rot_x = dx * cos_cam - dz * sin_cam;
        double rot_z = dx * sin_cam + dz * cos_cam;
        
        // Isometric projection
        int screen_x = center_x + static_cast<int>((rot_x - rot_z) * cos_iso * scale);
        int screen_y = center_y - static_cast<int>((rot_x + rot_z) * sin_iso * scale + dy * scale);
        
        return QPoint(screen_x, screen_y);
    };
    
    // Render each geometry object
    int obj_index = 0;
    for (const auto& geom_id : rendered_geometry_ids_) {
        bool is_selected = std::find(selected_objects_.begin(), selected_objects_.end(), geom_id) != selected_objects_.end();
        bool is_highlighted = std::find(highlighted_objects_.begin(), highlighted_objects_.end(), geom_id) != highlighted_objects_.end();
        
        QColor obj_color = QColor(150, 150, 200);
        if (is_selected) {
            obj_color = QColor(255, 255, 0);
        } else if (is_highlighted) {
            obj_color = QColor(0, 255, 255);
        }
        
        // Render simple 3D representation based on geometry type
        // For now, render as wireframe boxes/circles
        double offset_x = (obj_index % 5) * 2.0 - 4.0;
        double offset_y = (obj_index / 5) * 2.0 - 4.0;
        double offset_z = 0.0;
        
        if (display_mode_ == DisplayMode::Wireframe || display_mode_ == DisplayMode::HiddenLine) {
            // Draw wireframe box
            painter.setPen(QPen(obj_color, 1));
            
            // Bottom face
            QPoint p1 = project3D(offset_x, offset_y, offset_z);
            QPoint p2 = project3D(offset_x + 1.0, offset_y, offset_z);
            QPoint p3 = project3D(offset_x + 1.0, offset_y + 1.0, offset_z);
            QPoint p4 = project3D(offset_x, offset_y + 1.0, offset_z);
            painter.drawLine(p1, p2);
            painter.drawLine(p2, p3);
            painter.drawLine(p3, p4);
            painter.drawLine(p4, p1);
            
            // Top face
            QPoint p5 = project3D(offset_x, offset_y, offset_z + 1.0);
            QPoint p6 = project3D(offset_x + 1.0, offset_y, offset_z + 1.0);
            QPoint p7 = project3D(offset_x + 1.0, offset_y + 1.0, offset_z + 1.0);
            QPoint p8 = project3D(offset_x, offset_y + 1.0, offset_z + 1.0);
            painter.drawLine(p5, p6);
            painter.drawLine(p6, p7);
            painter.drawLine(p7, p8);
            painter.drawLine(p8, p5);
            
            // Vertical edges
            painter.drawLine(p1, p5);
            painter.drawLine(p2, p6);
            painter.drawLine(p3, p7);
            painter.drawLine(p4, p8);
        } else {
            // Shaded mode: draw filled polygon
            QPoint p1 = project3D(offset_x, offset_y, offset_z);
            QPoint p2 = project3D(offset_x + 1.0, offset_y, offset_z);
            QPoint p3 = project3D(offset_x + 1.0, offset_y + 1.0, offset_z);
            QPoint p4 = project3D(offset_x, offset_y + 1.0, offset_z);
            
            QPolygon front_face;
            front_face << p1 << p2 << p3 << p4;
            
            QColor fill_color = obj_color;
            fill_color.setAlpha(180);
            painter.setBrush(QBrush(fill_color));
            painter.setPen(QPen(obj_color.darker(), 1));
            painter.drawPolygon(front_face);
        }
        
        obj_index++;
    }
    
    // Show status info
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
    
    painter.setPen(QPen(QColor(200, 200, 200), 1));
    painter.drawText(10, height() - 40, QString("Objects: %1 | Mode: %2").arg(rendered_geometry_ids_.size()).arg(display_mode_str));
    
    if (!selected_objects_.empty()) {
        painter.setPen(QPen(QColor(255, 255, 0), 1));
        painter.drawText(10, height() - 25, QString("Selected: %1").arg(selected_objects_.size()));
    }
    
    if (!highlighted_objects_.empty()) {
        painter.setPen(QPen(QColor(0, 255, 255), 1));
        painter.drawText(10, height() - 10, QString("Highlighted: %1").arg(highlighted_objects_.size()));
    }
}

GeometryData Viewport3D::createGeometryData(const std::string& geometry_id, void* geometry_handle) const {
    GeometryData data;
    data.id = geometry_id;
    data.type = GeometryData::Box;
    data.params[0] = 1.0;
    data.params[1] = 1.0;
    data.params[2] = 0.5;
    data.params[3] = 1.0;
    data.params[4] = 0.0;
    data.params[5] = 0.0;
    data.params[6] = 0.0;
    data.params[7] = static_cast<double>(display_mode_);
    data.native_handle = geometry_handle;
    return data;
}

void Viewport3D::addGeometryToScene(const std::string& geometry_id, const GeometryData& data) {
    if (!render_engine_ || !render_engine_->isInitialized()) {
        return;
    }
    
    double identity[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
    render_engine_->addToScene(geometry_id, identity);
}

void Viewport3D::removeGeometryFromScene(const std::string& geometry_id) {
    if (render_engine_ && render_engine_->isInitialized()) {
        render_engine_->removeFromScene(geometry_id);
    }
}

std::string Viewport3D::pickObjectAt(int x, int y) const {
    if (useSoQtViewer()) {
        if (soqt_viewer_) {
            return soqt_viewer_->pickObject(x, y);
        }
    } else {
        if (render_engine_ && render_engine_->isInitialized()) {
            return render_engine_->pickObject(x, y);
        }
    }
    return {};
}

void Viewport3D::updateDisplayMode() {
    if (useSoQtViewer()) {
        if (soqt_viewer_) {
            int mode_int = static_cast<int>(display_mode_);
            soqt_viewer_->setDisplayMode(mode_int);
        }
    } else {
        if (!render_engine_ || !render_engine_->isInitialized()) {
            return;
        }
        
        int mode = static_cast<int>(display_mode_);
        for (const auto& geom_id : rendered_geometry_ids_) {
            render_engine_->setDisplayMode(geom_id, mode);
        }
    }
}

void Viewport3D::convertGeometryHandleToData(void* handle, GeometryData& data) const {
    if (!handle) {
        return;
    }
    
    uintptr_t handle_val = reinterpret_cast<uintptr_t>(handle);
    data.type = static_cast<GeometryData::Type>((handle_val % 6));
    data.params[0] = static_cast<double>((handle_val % 100) / 10.0);
    data.params[1] = static_cast<double>((handle_val % 200) / 20.0);
    data.params[2] = static_cast<double>((handle_val % 300) / 30.0);
    data.params[4] = static_cast<double>((handle_val % 50) - 25);
    data.params[5] = static_cast<double>((handle_val % 100) - 50);
    data.params[6] = static_cast<double>((handle_val % 150) - 75);
}

bool Viewport3D::useSoQtViewer() const {
#ifdef CAD_USE_COIN3D
#ifdef CAD_USE_QT
    return soqt_viewer_ != nullptr;
#else
    return false;
#endif
#else
    return false;
#endif
}

}  // namespace ui
}  // namespace cad

