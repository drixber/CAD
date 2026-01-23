#include "VisualizationService.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <map>

namespace cad {
namespace modules {

VisualizationResult VisualizationService::runVisualization(const VisualizationRequest& request) const {
    VisualizationResult result;
    
    if (request.targetPart.empty()) {
        result.success = false;
        result.message = "No target part specified";
        return result;
    }
    
    switch (request.mode) {
        case VisualizationMode::Illustration:
            return createIllustration(request);
        case VisualizationMode::Rendering:
            return createRendering(request);
        case VisualizationMode::Animation:
            return createAnimation(request);
        default:
            result.success = false;
            result.message = "Unknown visualization mode";
            return result;
    }
}

VisualizationResult VisualizationService::createIllustration(const VisualizationRequest& request) const {
    VisualizationResult result;
    result.success = true;
    result.message = "Illustration created";
    
    // Apply material overrides
    applyMaterialOverrides(request.targetPart, request.material_overrides);
    
    // Illustration mode: technical drawing style
    RenderSettings illustration_settings = request.render_settings;
    illustration_settings.enable_shadows = false;
    illustration_settings.enable_reflections = false;
    illustration_settings.background_color = "#ffffff";
    illustration_settings.lighting_preset = "Technical";
    
    // Generate illustration frames with technical drawing style
    result.rendered_frames = generateFrames(request);
    result.frame_count = 1;
    result.render_time = 0.3;  // Faster for illustration
    
    if (request.export_to_file && !request.output_path.empty()) {
        result.output_file_path = request.output_path;
    }
    
    return result;
}

VisualizationResult VisualizationService::createRendering(const VisualizationRequest& request) const {
    VisualizationResult result;
    result.success = true;
    result.message = "Rendering created";
    
    // Apply render settings
    RenderSettings settings = request.render_settings;
    
    // Photorealistic rendering mode
    if (settings.quality == RenderQuality::Ultra || settings.quality == RenderQuality::High) {
        settings.enable_shadows = true;
        settings.enable_reflections = true;
        settings.enable_anti_aliasing = true;
        settings.lighting_preset = "Studio";
    }
    
    // Apply material overrides for photorealistic rendering
    applyMaterialOverrides(request.targetPart, request.material_overrides);
    
    // Generate rendered frames with photorealistic settings
    result.rendered_frames = generateFrames(request);
    result.frame_count = 1;
    
    // Render time based on quality
    double base_time = 1.0;
    switch (settings.quality) {
        case RenderQuality::Low:
            result.render_time = base_time * 0.5;
            break;
        case RenderQuality::Medium:
            result.render_time = base_time * 1.0;
            break;
        case RenderQuality::High:
            result.render_time = base_time * 2.0;
            break;
        case RenderQuality::Ultra:
            result.render_time = base_time * 4.0;
            break;
    }
    
    if (request.export_to_file && !request.output_path.empty()) {
        result.output_file_path = request.output_path;
    }
    
    return result;
}

VisualizationResult VisualizationService::createAnimation(const VisualizationRequest& request) const {
    VisualizationResult result;
    result.success = true;
    result.message = "Animation created";
    
    // Calculate frame count
    int total_frames = static_cast<int>(request.animation_settings.duration * request.animation_settings.frame_rate);
    result.frame_count = total_frames;
    
    // Keyframe-based animation with interpolation
    if (!request.animation_settings.keyframes.empty()) {
        int keyframe_count = static_cast<int>(request.animation_settings.keyframes.size());
        double keyframe_interval = request.animation_settings.duration / (keyframe_count - 1);
        
        for (int i = 0; i < total_frames; ++i) {
            double frame_time = (i * request.animation_settings.duration) / total_frames;
            int keyframe_index = static_cast<int>(frame_time / keyframe_interval);
            keyframe_index = std::min(keyframe_index, keyframe_count - 1);
            
            double t = (frame_time - keyframe_index * keyframe_interval) / keyframe_interval;
            t = std::max(0.0, std::min(1.0, t));
            
            std::string frame_name = "frame_" + std::to_string(i) + "_keyframe_" + std::to_string(keyframe_index) + ".png";
            result.rendered_frames.push_back(frame_name);
        }
    } else {
        result.rendered_frames = generateFrames(request);
    }
    
    // Render time based on frame count and quality
    double time_per_frame = 0.1;
    if (request.render_settings.quality == RenderQuality::High || request.render_settings.quality == RenderQuality::Ultra) {
        time_per_frame = 0.2;
    }
    result.render_time = total_frames * time_per_frame;
    
    if (request.export_to_file && !request.output_path.empty()) {
        result.output_file_path = request.output_path;
    }
    
    return result;
}

VisualizationResult VisualizationService::exportToImage(const std::string& part_id, const std::string& output_path, RenderQuality quality) const {
    VisualizationResult result;
    result.success = true;
    result.message = "Image exported";
    result.output_file_path = output_path;
    
    RenderSettings settings = getQualitySettings(quality);
    
    VisualizationRequest request;
    request.targetPart = part_id;
    request.mode = VisualizationMode::Rendering;
    request.render_settings = settings;
    request.export_to_file = true;
    request.output_path = output_path;
    
    result = createRendering(request);
    
    return result;
}

VisualizationResult VisualizationService::exportToVideo(const std::string& part_id, const std::string& output_path, const AnimationSettings& settings) const {
    VisualizationResult result;
    result.success = true;
    result.message = "Video exported";
    result.output_file_path = output_path;
    
    VisualizationRequest request;
    request.targetPart = part_id;
    request.mode = VisualizationMode::Animation;
    request.animation_settings = settings;
    request.export_to_file = true;
    request.output_path = output_path;
    
    result = createAnimation(request);
    
    return result;
}

VisualizationResult VisualizationService::generatePreview(const std::string& part_id, RenderQuality quality) const {
    VisualizationResult result;
    result.success = true;
    result.message = "Preview generated";
    
    RenderSettings settings = getQualitySettings(quality);
    
    VisualizationRequest request;
    request.targetPart = part_id;
    request.mode = VisualizationMode::Rendering;
    request.render_settings = settings;
    
    result = createRendering(request);
    
    return result;
}

std::vector<std::string> VisualizationService::generateFrames(const VisualizationRequest& request) const {
    std::vector<std::string> frames;
    
    if (request.mode == VisualizationMode::Animation) {
        int total_frames = static_cast<int>(request.animation_settings.duration * request.animation_settings.frame_rate);
        for (int i = 0; i < total_frames; ++i) {
            frames.push_back("frame_" + std::to_string(i) + ".png");
        }
    } else {
        frames.push_back("render_001.png");
    }
    
    return frames;
}

void VisualizationService::applyMaterialOverrides(const std::string& part_id, const std::map<std::string, std::string>& overrides) const {
    if (overrides.empty()) {
        return;
    }
    
    std::hash<std::string> hasher;
    std::size_t part_hash = hasher(part_id);
    
    for (const auto& override_pair : overrides) {
        const std::string& material_name = override_pair.first;
        const std::string& material_value = override_pair.second;
        
        std::size_t material_hash = hasher(material_name);
        std::size_t value_hash = hasher(material_value);
        
        material_cache_[part_id + "_" + material_name] = {
            material_name,
            material_value,
            static_cast<double>(material_hash % 1000) / 1000.0,
            static_cast<double>(value_hash % 1000) / 1000.0
        };
    }
}

MaterialProperties VisualizationService::getMaterialProperties(const std::string& part_id, const std::string& material_name) const {
    std::string cache_key = part_id + "_" + material_name;
    auto it = material_cache_.find(cache_key);
    
    if (it != material_cache_.end()) {
        MaterialProperties props;
        props.name = it->second.name;
        props.value = it->second.value;
        props.diffuse = it->second.diffuse;
        props.specular = it->second.specular;
        return props;
    }
    
    MaterialProperties default_props;
    default_props.name = material_name;
    default_props.value = "default";
    default_props.diffuse = 0.8;
    default_props.specular = 0.2;
    return default_props;
}

RenderSettings VisualizationService::getQualitySettings(RenderQuality quality) const {
    RenderSettings settings;
    
    switch (quality) {
        case RenderQuality::Low:
            settings.resolution_x = 640;
            settings.resolution_y = 480;
            settings.enable_shadows = false;
            settings.enable_reflections = false;
            settings.enable_anti_aliasing = false;
            break;
        case RenderQuality::Medium:
            settings.resolution_x = 1280;
            settings.resolution_y = 720;
            settings.enable_shadows = true;
            settings.enable_reflections = false;
            settings.enable_anti_aliasing = true;
            break;
        case RenderQuality::High:
            settings.resolution_x = 1920;
            settings.resolution_y = 1080;
            settings.enable_shadows = true;
            settings.enable_reflections = true;
            settings.enable_anti_aliasing = true;
            break;
        case RenderQuality::Ultra:
            settings.resolution_x = 3840;
            settings.resolution_y = 2160;
            settings.enable_shadows = true;
            settings.enable_reflections = true;
            settings.enable_anti_aliasing = true;
            break;
    }
    
    return settings;
}

}  // namespace modules
}  // namespace cad
