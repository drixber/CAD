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
    
    int step_count = static_cast<int>(request.duration / request.time_step);
    if (step_count <= 0) {
        step_count = 100;
    }
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
    
    int iteration_count = 10;
    if (request.duration > 0.0) {
        iteration_count = static_cast<int>(request.duration * 10.0);
        iteration_count = std::max(5, std::min(iteration_count, 50));
    }
    
    auto mesh_it = mesh_element_counts_.find(request.targetAssembly);
    int element_count = (mesh_it != mesh_element_counts_.end()) ? mesh_it->second : 1000;
    
    double computation_time = 0.001 * element_count * iteration_count;
    computation_time = std::max(0.5, std::min(computation_time, 20.0));
    result.computation_time = computation_time;
    
    return result;
}

SimulationResult SimulationService::runThermalAnalysis(const SimulationRequest& request) const {
    SimulationResult result;
    result.success = true;
    result.message = "Thermal analysis completed";
    
    double max_temperature = 0.0;
    double min_temperature = 0.0;
    double thermal_flux = 0.0;
    
    for (const auto& constraint : request.constraints) {
        if (constraint.type == "Temperature") {
            max_temperature = std::max(max_temperature, constraint.value);
            min_temperature = std::min(min_temperature, constraint.value);
        }
    }
    
    double thermal_conductivity = 50.0;
    auto prop_it = request.material_properties.find("thermal_conductivity");
    if (prop_it != request.material_properties.end()) {
        thermal_conductivity = prop_it->second;
    }
    
    double temperature_gradient = max_temperature - min_temperature;
    thermal_flux = thermal_conductivity * temperature_gradient;
    
    auto mesh_it = mesh_element_counts_.find(request.targetAssembly);
    int element_count = (mesh_it != mesh_element_counts_.end()) ? mesh_it->second : 500;
    
    result.fea_result.max_stress = thermal_flux * 0.01;
    result.fea_result.max_displacement = temperature_gradient * 0.0001;
    result.fea_result.safety_factor = (max_temperature > 0.0) ? 1000.0 / max_temperature : 0.0;
    
    for (int i = 0; i < std::min(element_count, 100); ++i) {
        std::string node_id = "node_" + std::to_string(i);
        double temp = min_temperature + (max_temperature - min_temperature) * (static_cast<double>(i) / 100.0);
        result.fea_result.stress_map[node_id] = temp;
        result.fea_result.displacement_map[node_id] = temp * 0.0001;
    }
    
    double computation_time = 0.0005 * element_count;
    computation_time = std::max(0.1, std::min(computation_time, 5.0));
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
    std::hash<std::string> hasher;
    std::size_t part_hash = hasher(part_id);
    
    double base_size = 50.0 + static_cast<double>(part_hash % 200);
    double volume_estimate = base_size * base_size * base_size;
    
    if (element_size <= 0.0) {
        element_size = base_size / 20.0;
    }
    
    double element_volume = element_size * element_size * element_size;
    std::size_t estimated_elements = static_cast<std::size_t>(volume_estimate / element_volume);
    estimated_elements = std::max(estimated_elements, static_cast<std::size_t>(100));
    estimated_elements = std::min(estimated_elements, static_cast<std::size_t>(1000000));
    
    mesh_element_counts_[part_id] = estimated_elements;
    
    std::size_t node_count = estimated_elements * 4;
    if (estimated_elements > 10000) {
        node_count = estimated_elements * 8;
    }
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
    
    auto mesh_it = mesh_element_counts_.find(request.targetAssembly);
    std::size_t element_count = (mesh_it != mesh_element_counts_.end()) ? mesh_it->second : 1000;
    auto node_it = mesh_node_counts_.find(request.targetAssembly);
    std::size_t node_count = (node_it != mesh_node_counts_.end()) ? node_it->second : 500;
    
    double youngs_modulus = 200e9;
    double poissons_ratio = 0.3;
    double yield_strength = 250e6;
    double density = 7850.0;
    
    if (!request.material_properties.empty()) {
        auto it_e = request.material_properties.find("youngs_modulus");
        if (it_e != request.material_properties.end()) {
            youngs_modulus = it_e->second;
        }
        auto it_p = request.material_properties.find("poissons_ratio");
        if (it_p != request.material_properties.end()) {
            poissons_ratio = it_p->second;
        }
        auto it_y = request.material_properties.find("yield_strength");
        if (it_y != request.material_properties.end()) {
            yield_strength = it_y->second;
        }
        auto it_d = request.material_properties.find("density");
        if (it_d != request.material_properties.end()) {
            density = it_d->second;
        }
    }
    
    double total_force = 0.0;
    double total_pressure = 0.0;
    int fixed_count = 0;
    double max_temperature = 0.0;
    
    for (const auto& constraint : request.constraints) {
        if (constraint.type == "Fixed") {
            fixed_count++;
        } else if (constraint.type == "Force") {
            total_force += std::abs(constraint.value);
        } else if (constraint.type == "Pressure") {
            total_pressure += std::abs(constraint.value);
        } else if (constraint.type == "Temperature") {
            max_temperature = std::max(max_temperature, std::abs(constraint.value));
        }
    }
    
    if (fixed_count == 0) {
        fea.max_stress = 0.0;
        fea.max_displacement = 0.0;
        fea.safety_factor = 0.0;
        return fea;
    }
    
    double shear_modulus = youngs_modulus / (2.0 * (1.0 + poissons_ratio));
    double bulk_modulus = youngs_modulus / (3.0 * (1.0 - 2.0 * poissons_ratio));
    
    double effective_force = total_force;
    if (total_pressure > 0.0) {
        double area_estimate = static_cast<double>(element_count) * 0.01;
        effective_force += total_pressure * area_estimate;
    }
    
    double thermal_strain = 0.0;
    if (max_temperature > 0.0) {
        double thermal_expansion = 12e-6;
        thermal_strain = thermal_expansion * max_temperature;
    }
    
    double stress_scale = std::sqrt(static_cast<double>(element_count)) / 100.0;
    double displacement_scale = std::sqrt(static_cast<double>(node_count)) / 50.0;
    
    fea.max_stress = (effective_force / static_cast<double>(element_count)) * stress_scale * 1e6;
    if (thermal_strain > 0.0) {
        fea.max_stress += thermal_strain * youngs_modulus;
    }
    
    fea.max_displacement = (effective_force / youngs_modulus) * displacement_scale * 0.1;
    if (thermal_strain > 0.0) {
        fea.max_displacement += thermal_strain * 0.05;
    }
    
    if (fea.max_stress > 0.0) {
        fea.safety_factor = yield_strength / fea.max_stress;
    } else {
        fea.safety_factor = 0.0;
    }
    
    for (std::size_t i = 0; i < node_count && i < 100; ++i) {
        std::string node_id = "node_" + std::to_string(i);
        double node_factor = 0.5 + 0.5 * std::sin(static_cast<double>(i) * 0.1);
        fea.stress_map[node_id] = fea.max_stress * node_factor;
        fea.displacement_map[node_id] = fea.max_displacement * node_factor;
    }
    
    return fea;
}

MotionResult SimulationService::calculateMotion(const SimulationRequest& request) const {
    MotionResult motion;
    
    int step_count = static_cast<int>(request.duration / request.time_step);
    if (step_count <= 0) {
        step_count = 100;
    }
    step_count = std::min(step_count, 1000);
    
    double initial_position = 0.0;
    double initial_velocity = 0.0;
    double acceleration = 0.0;
    
    for (const auto& constraint : request.constraints) {
        if (constraint.type == "Force") {
            double mass = 1.0;
            auto prop_it = request.material_properties.find("density");
            if (prop_it != request.material_properties.end()) {
                mass = prop_it->second * 0.001;
            }
            acceleration = constraint.value / mass;
        }
    }
    
    motion.positions.reserve(step_count);
    motion.velocities.reserve(step_count);
    motion.accelerations.reserve(step_count);
    
    double current_position = initial_position;
    double current_velocity = initial_velocity;
    
    for (int i = 0; i < step_count; ++i) {
        double t = i * request.time_step;
        
        current_velocity = initial_velocity + acceleration * t;
        current_position = initial_position + initial_velocity * t + 0.5 * acceleration * t * t;
        
        motion.positions.push_back(current_position);
        motion.velocities.push_back(current_velocity);
        motion.accelerations.push_back(acceleration);
    }
    
    motion.simulation_time = request.duration;
    
    return motion;
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
    double total_force = 0.0;
    double beam_length = 1.0;
    double moment_of_inertia = 0.0001;
    
    for (const auto& constraint : request.constraints) {
        if (constraint.type == "Force") {
            total_force += std::abs(constraint.value);
        }
    }
    
    auto prop_it = request.material_properties.find("youngs_modulus");
    double youngs_modulus = (prop_it != request.material_properties.end()) ? prop_it->second : 200e9;
    
    if (request.duration > 0.0) {
        beam_length = request.duration;
    }
    
    double deflection = (total_force * beam_length * beam_length * beam_length) / (3.0 * youngs_modulus * moment_of_inertia);
    
    return deflection;
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
    SimulationRequest optimized = request;
    
    double best_safety_factor = 0.0;
    SimulationRequest best_request = request;
    
    int iteration_count = 10;
    if (request.duration > 0.0) {
        iteration_count = static_cast<int>(request.duration * 10.0);
        iteration_count = std::max(5, std::min(iteration_count, 50));
    }
    
    for (int i = 0; i < iteration_count; ++i) {
        double factor = 0.8 + (i * 0.4 / iteration_count);
        
        for (auto& constraint : optimized.constraints) {
            if (constraint.type == "Force") {
                constraint.value *= factor;
            }
        }
        
        FeaResult test_result = calculateFea(optimized);
        
        if (test_result.safety_factor > best_safety_factor) {
            best_safety_factor = test_result.safety_factor;
            best_request = optimized;
        }
    }
    
    return best_request;
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
