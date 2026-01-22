#pragma once

#include <string>
#include <vector>
#include <map>

namespace cad {
namespace modules {

enum class VisualizationMode {
    Illustration,
    Rendering,
    Animation
};

enum class RenderQuality {
    Low,
    Medium,
    High,
    Ultra
};

struct RenderSettings {
    RenderQuality quality{RenderQuality::Medium};
    int resolution_x{1920};
    int resolution_y{1080};
    bool enable_shadows{true};
    bool enable_reflections{true};
    bool enable_anti_aliasing{true};
    std::string background_color{"#2b2b2b"};
    std::string lighting_preset{"Studio"};
};

struct AnimationSettings {
    double duration{5.0};  // seconds
    int frame_rate{30};
    bool loop{false};
    std::vector<std::string> keyframes;
    std::string camera_path;
};

struct VisualizationRequest {
    std::string targetPart;
    VisualizationMode mode;
    RenderSettings render_settings;
    AnimationSettings animation_settings;
    std::map<std::string, std::string> material_overrides;
    bool export_to_file{false};
    std::string output_path;
};

struct VisualizationResult {
    bool success{false};
    std::string message;
    std::string output_file_path;
    int frame_count{0};
    double render_time{0.0};
    std::vector<std::string> rendered_frames;
};

class VisualizationService {
public:
    VisualizationResult runVisualization(const VisualizationRequest& request) const;
    VisualizationResult createIllustration(const VisualizationRequest& request) const;
    VisualizationResult createRendering(const VisualizationRequest& request) const;
    VisualizationResult createAnimation(const VisualizationRequest& request) const;
    
    // Export
    VisualizationResult exportToImage(const std::string& part_id, const std::string& output_path, RenderQuality quality) const;
    VisualizationResult exportToVideo(const std::string& part_id, const std::string& output_path, const AnimationSettings& settings) const;
    
    // Preview
    VisualizationResult generatePreview(const std::string& part_id, RenderQuality quality) const;
    
private:
    std::vector<std::string> generateFrames(const VisualizationRequest& request) const;
    void applyMaterialOverrides(const std::string& part_id, const std::map<std::string, std::string>& overrides) const;
    RenderSettings getQualitySettings(RenderQuality quality) const;
};

}  // namespace modules
}  // namespace cad
