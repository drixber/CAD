#pragma once

#include <string>
#include <vector>
#include <map>

namespace cad {
namespace modules {

enum class SimulationType {
    Motion,
    FEA,
    Deflection,
    Optimization
};

struct SimulationConstraint {
    std::string name;
    std::string type;  // "Fixed", "Force", "Pressure", "Temperature"
    double value{0.0};
    std::string location;
};

struct FeaResult {
    double max_stress{0.0};
    double max_displacement{0.0};
    double safety_factor{0.0};
    std::map<std::string, double> stress_map;
    std::map<std::string, double> displacement_map;
};

struct MotionResult {
    std::vector<double> positions;
    std::vector<double> velocities;
    std::vector<double> accelerations;
    double simulation_time{0.0};
};

struct SimulationRequest {
    std::string targetAssembly;
    SimulationType type;
    std::vector<SimulationConstraint> constraints;
    double time_step{0.01};
    double duration{1.0};
    std::map<std::string, double> material_properties;
};

struct SimulationResult {
    bool success{false};
    std::string message;
    FeaResult fea_result;
    MotionResult motion_result;
    std::vector<std::string> warnings;
    double computation_time{0.0};
};

class SimulationService {
public:
    SimulationResult runSimulation(const SimulationRequest& request) const;
    SimulationResult runFeaAnalysis(const SimulationRequest& request) const;
    SimulationResult runMotionAnalysis(const SimulationRequest& request) const;
    SimulationResult runDeflectionAnalysis(const SimulationRequest& request) const;
    SimulationResult runOptimization(const SimulationRequest& request) const;
    
    // Material properties
    std::map<std::string, double> getDefaultMaterialProperties(const std::string& material_name) const;
    void setMaterialProperties(const std::string& part_id, const std::map<std::string, double>& properties);
    
    // Mesh generation
    void generateMesh(const std::string& part_id, double element_size) const;
    std::size_t getMeshElementCount(const std::string& part_id) const;
    
    // Results visualization
    std::vector<double> getStressValues(const std::string& part_id) const;
    std::vector<double> getDisplacementValues(const std::string& part_id) const;
    
private:
    mutable std::map<std::string, std::map<std::string, double>> material_properties_;
    mutable std::map<std::string, std::size_t> mesh_element_counts_;
    
    FeaResult calculateFea(const SimulationRequest& request) const;
    MotionResult calculateMotion(const SimulationRequest& request) const;
    double calculateDeflection(const SimulationRequest& request) const;
    SimulationRequest optimizeParameters(const SimulationRequest& request) const;
};

}  // namespace modules
}  // namespace cad
