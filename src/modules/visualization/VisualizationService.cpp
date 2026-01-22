#include "VisualizationService.h"

#include <algorithm>
#include <cmath>

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
    
    // Generate illustration frames
    result.rendered_frames = generateFrames(request);
    result.frame_count = 1;
    result.render_time = 0.5;
    
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
    
    // Generate rendered frames
    result.rendered_frames = generateFrames(request);
    result.frame_count = 1;
    result.render_time = 2.0;  // Rendering takes longer
    
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
    
    // Generate animation frames
    result.rendered_frames = generateFrames(request);
    result.render_time = total_frames * 0.1;  // 0.1s per frame
    
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
    // In real implementation: would apply material overrides to part
    (void)part_id;
    (void)overrides;
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
