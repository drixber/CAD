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
    
    // Generate mesh if not exists
    if (mesh_element_counts_.find(request.targetAssembly) == mesh_element_counts_.end()) {
        generateMesh(request.targetAssembly, 1.0);
    }
    
    result.computation_time = 0.5;  // Simulated computation time
    
    return result;
}

SimulationResult SimulationService::runMotionAnalysis(const SimulationRequest& request) const {
    SimulationResult result;
    result.success = true;
    result.message = "Motion analysis completed";
    
    // Calculate motion results
    result.motion_result = calculateMotion(request);
    
    result.computation_time = 0.3;  // Simulated computation time
    
    return result;
}

SimulationResult SimulationService::runDeflectionAnalysis(const SimulationRequest& request) const {
    SimulationResult result;
    result.success = true;
    result.message = "Deflection analysis completed";
    
    double deflection = calculateDeflection(request);
    result.fea_result.max_displacement = deflection;
    
    result.computation_time = 0.2;  // Simulated computation time
    
    return result;
}

SimulationResult SimulationService::runOptimization(const SimulationRequest& request) const {
    SimulationResult result;
    result.success = true;
    result.message = "Optimization completed";
    
    // Optimize parameters
    SimulationRequest optimized = optimizeParameters(request);
    
    // Run FEA on optimized design
    result.fea_result = calculateFea(optimized);
    
    result.computation_time = 1.0;  // Simulated computation time
    
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
    // In real implementation: generate finite element mesh using OCCT or external tool
    // BRepMesh_IncrementalMesh mesh(shape, element_size, false, 0.5, true);
    // mesh.Perform();
    // Get mesh data and store element count
    // For now, estimate element count based on part size
    std::size_t estimated_elements = static_cast<std::size_t>(100.0 / element_size);
    mesh_element_counts_[part_id] = estimated_elements;
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
    // In real implementation with FEA solver:
    // std::vector<double> stresses;
    // for (const auto& node : mesh_nodes) {
    //     double stress = fea_solver->getStressAtNode(node);
    //     stresses.push_back(stress);
    // }
    // return stresses;
    
    // For now: return simulated stress values based on part ID hash
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
    // Return displacement values from FEA results
    // In real implementation with FEA solver:
    // std::vector<double> displacements;
    // for (const auto& node : mesh_nodes) {
    //     double displacement = fea_solver->getDisplacementAtNode(node);
    //     displacements.push_back(displacement);
    // }
    // return displacements;
    
    // For now: return simulated displacement values based on part ID hash
    std::vector<double> displacements;
    std::hash<std::string> hasher;
    std::size_t hash = hasher(part_id);
    for (int i = 0; i < 5; ++i) {
        double base_disp = 0.001 + (hash % 100) * 0.00001;
        displacements.push_back(base_disp + i * 0.0005);
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
