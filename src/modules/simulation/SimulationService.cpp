#include "SimulationService.h"

#include <algorithm>
#include <cmath>

namespace cad {
namespace modules {

SimulationResult SimulationService::runSimulation(const SimulationRequest& request) const {
    SimulationResult result;
    
    if (request.targetAssembly.empty()) {
        result.success = false;
        result.message = "No target assembly specified";
        return result;
    }
    
    switch (request.type) {
        case SimulationType::FEA:
            return runFeaAnalysis(request);
        case SimulationType::Motion:
            return runMotionAnalysis(request);
        case SimulationType::Deflection:
            return runDeflectionAnalysis(request);
        case SimulationType::Optimization:
            return runOptimization(request);
        default:
            result.success = false;
            result.message = "Unknown simulation type";
            return result;
    }
}

SimulationResult SimulationService::runFeaAnalysis(const SimulationRequest& request) const {
    SimulationResult result;
    result.success = true;
    result.message = "FEA analysis completed";
    
    // Calculate FEA results
    result.fea_result = calculateFea(request);
    
    if (mesh_element_counts_.find(request.targetAssembly) == mesh_element_counts_.end()) {
        generateMesh(request.targetAssembly, 1.0);
    }
    
    auto mesh_it = mesh_element_counts_.find(request.targetAssembly);
    int element_count = (mesh_it != mesh_element_counts_.end()) ? mesh_it->second : 1000;
    
    double computation_time = 0.001 * element_count;
    computation_time = std::max(0.1, std::min(computation_time, 10.0));
    result.computation_time = computation_time;
    
    return result;
}

SimulationResult SimulationService::runMotionAnalysis(const SimulationRequest& request) const {
    SimulationResult result;
    result.success = true;
    result.message = "Motion analysis completed";
    
    result.motion_result = calculateMotion(request);
    
    int step_count = request.motion_params.step_count > 0 ? request.motion_params.step_count : 100;
    double computation_time = 0.003 * step_count;
    computation_time = std::max(0.1, std::min(computation_time, 5.0));
    result.computation_time = computation_time;
    
    return result;
}

SimulationResult SimulationService::runDeflectionAnalysis(const SimulationRequest& request) const {
    SimulationResult result;
    result.success = true;
    result.message = "Deflection analysis completed";
    
    double deflection = calculateDeflection(request);
    result.fea_result.max_displacement = deflection;
    
    auto mesh_it = mesh_element_counts_.find(request.targetAssembly);
    int element_count = (mesh_it != mesh_element_counts_.end()) ? mesh_it->second : 500;
    
    double computation_time = 0.0004 * element_count;
    computation_time = std::max(0.05, std::min(computation_time, 3.0));
    result.computation_time = computation_time;
    
    return result;
}

SimulationResult SimulationService::runOptimization(const SimulationRequest& request) const {
    SimulationResult result;
    result.success = true;
    result.message = "Optimization completed";
    
    SimulationRequest optimized = optimizeParameters(request);
    
    result.fea_result = calculateFea(optimized);
    
    int iteration_count = request.optimization_params.max_iterations > 0 ? 
                         request.optimization_params.max_iterations : 10;
    
    auto mesh_it = mesh_element_counts_.find(request.targetAssembly);
    int element_count = (mesh_it != mesh_element_counts_.end()) ? mesh_it->second : 1000;
    
    double computation_time = 0.001 * element_count * iteration_count;
    computation_time = std::max(0.5, std::min(computation_time, 20.0));
    result.computation_time = computation_time;
    
    return result;
}

std::map<std::string, double> SimulationService::getDefaultMaterialProperties(const std::string& material_name) const {
    std::map<std::string, double> properties;
    
    if (material_name == "Steel") {
        properties["density"] = 7850.0;  // kg/m³
        properties["youngs_modulus"] = 200e9;  // Pa
        properties["poissons_ratio"] = 0.3;
        properties["yield_strength"] = 250e6;  // Pa
    } else if (material_name == "Aluminum") {
        properties["density"] = 2700.0;
        properties["youngs_modulus"] = 70e9;
        properties["poissons_ratio"] = 0.33;
        properties["yield_strength"] = 275e6;
    } else if (material_name == "Plastic") {
        properties["density"] = 1200.0;
        properties["youngs_modulus"] = 3e9;
        properties["poissons_ratio"] = 0.4;
        properties["yield_strength"] = 50e6;
    }
    
    return properties;
}

void SimulationService::setMaterialProperties(const std::string& part_id, const std::map<std::string, double>& properties) {
    material_properties_[part_id] = properties;
}

void SimulationService::generateMesh(const std::string& part_id, double element_size) const {
    double base_size = 100.0;
    double volume_estimate = base_size * base_size * base_size;
    double element_volume = element_size * element_size * element_size;
    
    std::size_t estimated_elements = static_cast<std::size_t>(volume_estimate / element_volume);
    estimated_elements = std::max(estimated_elements, static_cast<std::size_t>(100));
    estimated_elements = std::min(estimated_elements, static_cast<std::size_t>(1000000));
    
    mesh_element_counts_[part_id] = estimated_elements;
    
    std::size_t node_count = estimated_elements * 4;
    mesh_node_counts_[part_id] = node_count;
}

std::size_t SimulationService::getMeshElementCount(const std::string& part_id) const {
    auto it = mesh_element_counts_.find(part_id);
    if (it != mesh_element_counts_.end()) {
        return it->second;
    }
    return 0;
}

std::vector<double> SimulationService::getStressValues(const std::string& part_id) const {
    // Return stress values from FEA results
    std::vector<double> stresses;
    
    auto mesh_it = mesh_node_counts_.find(part_id);
    int node_count = (mesh_it != mesh_node_counts_.end()) ? mesh_it->second : 100;
    
    std::hash<std::string> hasher;
    std::size_t part_hash = hasher(part_id);
    
    for (int i = 0; i < node_count; ++i) {
        std::size_t node_hash = part_hash + static_cast<std::size_t>(i);
        double base_stress = static_cast<double>(node_hash % 100000) / 1000.0;
        double variation = static_cast<double>((node_hash / 100000) % 100) / 10.0;
        double stress = base_stress + variation;
        stresses.push_back(stress);
    }
    
    return stresses;
    std::vector<double> stresses;
    std::hash<std::string> hasher;
    std::size_t hash = hasher(part_id);
    for (int i = 0; i < 5; ++i) {
        double base_stress = 100e6 + (hash % 100) * 1e6;
        stresses.push_back(base_stress + i * 10e6);
    }
    return stresses;
}

std::vector<double> SimulationService::getDisplacementValues(const std::string& part_id) const {
    std::vector<double> displacements;
    
    auto node_it = mesh_node_counts_.find(part_id);
    std::size_t node_count = (node_it != mesh_node_counts_.end()) ? node_it->second : 100;
    
    std::hash<std::string> hasher;
    std::size_t hash = hasher(part_id);
    
    auto prop_it = material_properties_.find(part_id);
    double youngs_modulus = 200e9;
    if (prop_it != material_properties_.end()) {
        auto young_it = prop_it->second.find("youngs_modulus");
        if (young_it != prop_it->second.end()) {
            youngs_modulus = young_it->second;
        }
    }
    
    double max_displacement = 0.01;
    double scale_factor = 1.0 / (youngs_modulus / 1e9);
    
    for (std::size_t i = 0; i < node_count && i < 1000; ++i) {
        double base_disp = max_displacement * scale_factor * (0.1 + (hash % 100) * 0.001);
        double variation = std::cos(static_cast<double>(i) * 0.15) * max_displacement * 0.05 * scale_factor;
        displacements.push_back(base_disp + variation);
    }
    
    return displacements;
}

FeaResult SimulationService::calculateFea(const SimulationRequest& request) const {
    FeaResult fea;
    
    // Simplified FEA calculation
    double max_force = 0.0;
    for (const auto& constraint : request.constraints) {
        if (constraint.type == "Force") {
            max_force = std::max(max_force, std::abs(constraint.value));
        }
    }
    
    // Estimate stress based on force and material properties
    double youngs_modulus = 200e9;  // Default steel
    if (!request.material_properties.empty()) {
        auto it = request.material_properties.find("youngs_modulus");
        if (it != request.material_properties.end()) {
            youngs_modulus = it->second;
        }
    }
    
    fea.max_stress = max_force * 1e6;  // Simplified calculation
    fea.max_displacement = max_force / youngs_modulus * 0.1;  // Simplified calculation
    fea.safety_factor = 250e6 / fea.max_stress;  // Yield strength / max stress
    
    // Populate stress and displacement maps
    fea.stress_map["node_1"] = fea.max_stress * 0.8;
    fea.stress_map["node_2"] = fea.max_stress * 0.9;
    fea.stress_map["node_3"] = fea.max_stress;
    
    fea.displacement_map["node_1"] = fea.max_displacement * 0.7;
    fea.displacement_map["node_2"] = fea.max_displacement * 0.85;
    fea.displacement_map["node_3"] = fea.max_displacement;
    
    return fea;
}

MotionResult SimulationService::calculateMotion(const SimulationRequest& request) const {
    MotionResult motion;
    
    // Simplified motion calculation
    int steps = static_cast<int>(request.duration / request.time_step);
    motion.positions.reserve(steps);
    motion.velocities.reserve(steps);
    motion.accelerations.reserve(steps);
    
    double initial_velocity = 0.0;
    double acceleration = 9.81;  // Gravity
    
    for (int i = 0; i < steps; ++i) {
        double t = i * request.time_step;
        double position = initial_velocity * t + 0.5 * acceleration * t * t;
        double velocity = initial_velocity + acceleration * t;
        
        motion.positions.push_back(position);
        motion.velocities.push_back(velocity);
        motion.accelerations.push_back(acceleration);
    }
    
    motion.simulation_time = request.duration;
    
    return motion;
}

double SimulationService::calculateDeflection(const SimulationRequest& request) const {
    // Simplified deflection calculation
    double max_force = 0.0;
    for (const auto& constraint : request.constraints) {
        if (constraint.type == "Force") {
            max_force = std::max(max_force, std::abs(constraint.value));
        }
    }
    
    double youngs_modulus = 200e9;
    if (!request.material_properties.empty()) {
        auto it = request.material_properties.find("youngs_modulus");
        if (it != request.material_properties.end()) {
            youngs_modulus = it->second;
        }
    }
    
    // Simplified beam deflection: δ = FL³ / (3EI)
    double length = 1.0;  // Assume 1m length
    double moment_of_inertia = 1e-6;  // Assume small I
    double deflection = (max_force * length * length * length) / (3.0 * youngs_modulus * moment_of_inertia);
    
    return deflection;
}

SimulationRequest SimulationService::optimizeParameters(const SimulationRequest& request) const {
    // Simplified optimization: reduce force by 10%
    SimulationRequest optimized = request;
    
    for (auto& constraint : optimized.constraints) {
        if (constraint.type == "Force") {
            constraint.value *= 0.9;  // Reduce force by 10%
        }
    }
    
    return optimized;
}

}  // namespace modules
}  // namespace cad
