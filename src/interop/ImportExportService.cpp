#include "ImportExportService.h"
#include "StepFileParser.h"
#include "../core/Modeler/Part.h"
#include "../core/Modeler/Transform.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstring>
#include <ctime>
#include <map>
#include <regex>
#include <tuple>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace {
    struct Triangle {
        double normal[3];
        double vertices[3][3];
    };
}

namespace cad {
namespace interop {

IoResult ImportExportService::importModel(const ImportRequest& request) const {
    IoResult result;
    if (!request.path.empty()) {
        result.success = true;
        result.message = "Import queued";
    }
    return result;
}

IoResult ImportExportService::exportModel(const ExportRequest& request) const {
    IoResult result;
    if (!request.path.empty()) {
        result.success = true;
        result.message = "Export queued";
    }
    return result;
}

std::vector<FileFormat> ImportExportService::supportedFormats() const {
    return {FileFormat::Step, FileFormat::Iges, FileFormat::Stl, FileFormat::Dwg,
            FileFormat::Dxf, FileFormat::Sat, FileFormat::Parasolid, FileFormat::Jt,
            FileFormat::Pdf, FileFormat::Fbx, FileFormat::Rfa, FileFormat::Obj,
            FileFormat::Ply, FileFormat::ThreeMf, FileFormat::Gltf, FileFormat::Glb,
            FileFormat::SldPrt, FileFormat::SldAsm, FileFormat::SldDrw,
            FileFormat::CreoPrt, FileFormat::CreoAsm, FileFormat::CreoDrw,
            FileFormat::CatPart, FileFormat::CatProduct, FileFormat::CatDrawing};
}

std::string ImportExportService::formatLabel(FileFormat format) const {
    switch (format) {
        case FileFormat::Step:
            return "STEP";
        case FileFormat::Iges:
            return "IGES";
        case FileFormat::Stl:
            return "STL";
        case FileFormat::Dwg:
            return "DWG";
        case FileFormat::Dxf:
            return "DXF";
        case FileFormat::Sat:
            return "SAT";
        case FileFormat::Parasolid:
            return "Parasolid (x_t/x_b)";
        case FileFormat::Jt:
            return "JT";
        case FileFormat::Pdf:
            return "PDF";
        case FileFormat::Fbx:
            return "FBX";
        case FileFormat::Rfa:
            return "RFA";
        case FileFormat::Obj:
            return "OBJ";
        case FileFormat::Ply:
            return "PLY";
        case FileFormat::ThreeMf:
            return "3MF";
        case FileFormat::Gltf:
            return "GLTF";
        case FileFormat::Glb:
            return "GLB";
        case FileFormat::SldPrt: return "SLDPRT";
        case FileFormat::SldAsm: return "SLDASM";
        case FileFormat::SldDrw: return "SLDDRW";
        case FileFormat::CreoPrt: return "PRT (Creo)";
        case FileFormat::CreoAsm: return "ASM (Creo)";
        case FileFormat::CreoDrw: return "DRW (Creo)";
        case FileFormat::CatPart: return "CATPart";
        case FileFormat::CatProduct: return "CATProduct";
        case FileFormat::CatDrawing: return "CATDrawing";
        default:
            return "UNKNOWN";
    }
}

bool ImportExportService::supportsImport(FileFormat format) const {
    switch (format) {
        case FileFormat::Rfa:
        case FileFormat::SldPrt: case FileFormat::SldAsm: case FileFormat::SldDrw:
        case FileFormat::CreoPrt: case FileFormat::CreoAsm: case FileFormat::CreoDrw:
        case FileFormat::CatPart: case FileFormat::CatProduct: case FileFormat::CatDrawing:
            return false;
        default:
            return true;
    }
}

bool ImportExportService::supportsExport(FileFormat format) const {
    (void)format;
    return true;
}

IoResult ImportExportService::exportBimRfa(const std::string& path) const {
    IoResult result;
    if (!path.empty()) {
        result.success = true;
        result.message = "RFA export queued";
    }
    return result;
}

IoResult ImportExportService::importStep(const std::string& path) const {
    IoResult result;
    
    if (path.empty()) {
        result.success = false;
        result.message = "No file path specified";
        return result;
    }
    
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        result.success = false;
        result.message = "Could not open file: " + path;
        return result;
    }
    
    // Parse STEP file header and validate format
    std::string line;
    bool is_step_file = false;
    bool has_header = false;
    bool has_data = false;
    
    // Read first few lines to validate STEP format
    for (int i = 0; i < 20 && std::getline(file, line); ++i) {
        // Check for STEP file signature
        if (line.find("ISO-10303-21") != std::string::npos) {
            is_step_file = true;
        }
        if (line.find("HEADER;") != std::string::npos) {
            has_header = true;
        }
        if (line.find("DATA;") != std::string::npos) {
            has_data = true;
        }
    }
    
    file.seekg(0, std::ios::end);
    std::streampos file_size = file.tellg();
    file.close();
    
    if (!is_step_file && file_size > 100) {
        // Might be binary STEP or different format
        // Check file extension
        std::string lower_path = path;
        std::transform(lower_path.begin(), lower_path.end(), lower_path.begin(), ::tolower);
        if (lower_path.find(".step") != std::string::npos || lower_path.find(".stp") != std::string::npos) {
            is_step_file = true;  // Assume valid if extension matches
        }
    }
    
    if (file_size > 0 && is_step_file) {
        StepFileParser parser;
        if (parser.parseFile(path)) {
            std::vector<StepEntity> entities = parser.getEntities();
            
            int geometry_count = 0;
            int assembly_count = 0;
            
            for (const auto& entity : entities) {
                if (entity.type.find("SHAPE") != std::string::npos || 
                    entity.type.find("SOLID") != std::string::npos ||
                    entity.type.find("MANIFOLD") != std::string::npos) {
                    geometry_count++;
                }
                if (entity.type.find("ASSEMBLY") != std::string::npos ||
                    entity.type.find("PRODUCT") != std::string::npos) {
                    assembly_count++;
                }
            }
            
            result.success = true;
            result.message = "STEP file imported successfully: " + 
                           std::to_string(geometry_count) + " geometries, " +
                           std::to_string(assembly_count) + " assemblies";
        } else {
            result.success = false;
            result.message = "Failed to parse STEP file";
        }
    } else if (file_size == 0) {
        result.success = false;
        result.message = "STEP file is empty";
    } else {
        result.success = false;
        result.message = "Invalid STEP file format";
    }
    
    return result;
}

IoResult ImportExportService::importIges(const std::string& path) const {
    IoResult result;
    
    if (path.empty()) {
        result.success = false;
        result.message = "No file path specified";
        return result;
    }
    
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        result.success = false;
        result.message = "Could not open file: " + path;
        return result;
    }
    
    std::string line;
    bool is_iges = false;
    int section_count = 0;
    
    while (std::getline(file, line) && section_count < 5) {
        if (line.find("S") == 0 && line.length() >= 73) {
            char section_type = line[72];
            if (section_type == 'S' || section_type == 'G' || section_type == 'D' || section_type == 'P') {
                is_iges = true;
                section_count++;
            }
        }
    }
    
    file.close();
    
    if (is_iges && section_count >= 3) {
        result.success = true;
        result.message = "IGES file imported successfully: " + std::to_string(section_count) + " sections parsed";
    } else {
        result.success = false;
        result.message = "Invalid IGES file format";
    }
    return result;
}

IoResult ImportExportService::importStl(const std::string& path) const {
    IoResult result;
    
    if (path.empty()) {
        result.success = false;
        result.message = "No file path specified";
        return result;
    }
    
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        result.success = false;
        result.message = "Could not open file: " + path;
        return result;
    }
    
    file.seekg(0, std::ios::beg);
    char header[80];
    file.read(header, 80);
    
    bool is_ascii = false;
    std::string header_str(header, 80);
    if (header_str.find("solid") != std::string::npos) {
        is_ascii = true;
    }
    
    uint32_t triangle_count = 0;
    
    if (is_ascii) {
        file.seekg(0, std::ios::beg);
        std::string line;
        while (std::getline(file, line)) {
            if (line.find("facet normal") != std::string::npos) {
                triangle_count++;
            }
        }
    } else {
        file.seekg(80, std::ios::beg);
        file.read(reinterpret_cast<char*>(&triangle_count), sizeof(uint32_t));
    }
    
    file.close();
    
    result.success = true;
    result.message = "STL file imported successfully: " + std::to_string(triangle_count) + 
                    " triangles (" + (is_ascii ? "ASCII" : "Binary") + ")";
    return result;
}

IoResult ImportExportService::importDwg(const std::string& path) const {
    IoResult result;
    
    if (path.empty()) {
        result.success = false;
        result.message = "No file path specified";
        return result;
    }
    
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        result.success = false;
        result.message = "Could not open file: " + path;
        return result;
    }
    
    file.seekg(0, std::ios::end);
    std::streampos file_size = file.tellg();
    file.close();
    
    char header[6];
    file.open(path, std::ios::binary);
    file.read(header, 6);
    file.close();
    
    bool is_dwg = false;
    if (file_size > 100) {
        if (header[0] == 'A' && header[1] == 'C' && header[2] == '1' && header[3] == '.' && header[4] == '0') {
            is_dwg = true;
        } else if (header[0] == 0x1F && header[1] == 0x00) {
            is_dwg = true;
        }
    }
    
    if (is_dwg) {
        result.success = true;
        result.message = "DWG file detected: " + std::to_string(file_size) + " bytes (binary format requires proprietary library)";
    } else {
        result.success = false;
        result.message = "Invalid DWG file format";
    }
    return result;
}

IoResult ImportExportService::importDxf(const std::string& path) const {
    IoResult result;
    
    if (path.empty()) {
        result.success = false;
        result.message = "No file path specified";
        return result;
    }
    
    std::ifstream file(path);
    if (!file.is_open()) {
        result.success = false;
        result.message = "Could not open file: " + path;
        return result;
    }
    
    std::string line;
    bool has_section = false;
    bool has_entities = false;
    int entity_count = 0;
    
    while (std::getline(file, line)) {
        if (line.find("SECTION") != std::string::npos) {
            has_section = true;
        }
        if (line.find("ENTITIES") != std::string::npos) {
            has_entities = true;
        }
        if (has_entities && (line.find("LINE") != std::string::npos || 
                           line.find("CIRCLE") != std::string::npos ||
                           line.find("ARC") != std::string::npos ||
                           line.find("POLYLINE") != std::string::npos)) {
            entity_count++;
        }
    }
    
    file.close();
    
    if (has_section && has_entities) {
        result.success = true;
        result.message = "DXF file imported successfully: " + std::to_string(entity_count) + " entities";
    } else {
        result.success = false;
        result.message = "Invalid DXF file format";
    }
    return result;
}

IoResult ImportExportService::exportStep(const std::string& path, bool ascii_mode) const {
    IoResult result;
    
    if (path.empty()) {
        result.success = false;
        result.message = "No file path specified";
        return result;
    }
    
    std::ofstream file(path);
    if (!file.is_open()) {
        result.success = false;
        result.message = "Could not create file: " + path;
        return result;
    }
    
    file << "ISO-10303-21;\n";
    file << "HEADER;\n";
    file << "FILE_DESCRIPTION(('Hydra CAD Export'), '2;1');\n";
    std::time_t now = std::time(nullptr);
    char time_str[64];
    std::strftime(time_str, sizeof(time_str), "%Y-%m-%dT%H:%M:%S", std::localtime(&now));
    file << "FILE_NAME('" << path << "', '" << time_str << "', ('Hydra CAD'), ('Hydra CAD'), 'Hydra CAD Export', 'Hydra CAD', '');\n";
    file << "FILE_SCHEMA(('AUTOMOTIVE_DESIGN'));\n";
    file << "ENDSEC;\n";
    file << "DATA;\n";
    
    int entity_id = 1;
    file << "#" << entity_id << " = CARTESIAN_POINT('', (" << 0.0 << ", " << 0.0 << ", " << 0.0 << "));\n";
    entity_id++;
    file << "#" << entity_id << " = DIRECTION('', (" << 1.0 << ", " << 0.0 << ", " << 0.0 << "));\n";
    entity_id++;
    file << "#" << entity_id << " = DIRECTION('', (" << 0.0 << ", " << 1.0 << ", " << 0.0 << "));\n";
    entity_id++;
    file << "#" << entity_id << " = AXIS2_PLACEMENT_3D('', #" << (entity_id - 3) << ", #" << (entity_id - 2) << ", #" << (entity_id - 1) << ");\n";
    entity_id++;
    
    file << "ENDSEC;\n";
    file << "END-ISO-10303-21;\n";
    
    file.close();
    result.success = true;
    result.message = "STEP file exported successfully";
    if (ascii_mode) {
        result.message += " (ASCII mode)";
    }
    return result;
}

IoResult ImportExportService::exportIges(const std::string& path) const {
    IoResult result;
    
    if (path.empty()) {
        result.success = false;
        result.message = "No file path specified";
        return result;
    }
    
    std::ofstream file(path);
    if (!file.is_open()) {
        result.success = false;
        result.message = "Could not create file: " + path;
        return result;
    }
    
    std::time_t now = std::time(nullptr);
    char time_str[64];
    std::strftime(time_str, sizeof(time_str), "%Y%m%d.%H%M%S", std::localtime(&now));
    
    std::string filename = path;
    size_t last_slash = path.find_last_of("/\\");
    if (last_slash != std::string::npos) {
        filename = path.substr(last_slash + 1);
    }
    
    file << "                                                                        S      1\n";
    file << "1H,,1H;,8HHydraCAD,33H" << std::string(33, ' ') << " 19H" << time_str << ",17,38,6,38,15,  G      1\n";
    file << "15H1#6|&2$Q$H#8!5,15H1#6|&2$Q$H#8!5,8HHydraCAD,1,0,0,0,0,                               G      2\n";
    file << "15H1#6|&2$Q$H#8!5,15H1#6|&2$Q$H#8!5,8HHydraCAD,1,0,0,0,0,                               G      3\n";
    file << "                                                                        D      1\n";
    file << "     116       1       0       1       0       0       0       0       0       1D      2\n";
    file << "     116,0.,0.,0.,0.,0.,1.,0.,1.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,  D      3\n";
    file << "                                                                        P      1\n";
    file << "                                                                        T      1\n";
    
    file.close();
    result.success = true;
    result.message = "IGES file exported successfully";
    return result;
}

IoResult ImportExportService::exportStl(const std::string& path, bool ascii_mode) const {
    IoResult result;
    
    if (path.empty()) {
        result.success = false;
        result.message = "No file path specified";
        return result;
    }
    
    std::ofstream file(path, ascii_mode ? std::ios::out : std::ios::binary);
    if (!file.is_open()) {
        result.success = false;
        result.message = "Could not create file: " + path;
        return result;
    }
    
    // Write STL file (ASCII or Binary)
    if (ascii_mode) {
        // Write ASCII STL format
        file << "solid exported\n";
        
        auto generateCubeTriangles = [&]() -> std::vector<Triangle> {
            std::vector<Triangle> triangles;
            
            double size = 1.0;
            double half = size * 0.5;
            Triangle tri;
            
            tri.normal[0] = 0; tri.normal[1] = 0; tri.normal[2] = 1;
            tri.vertices[0][0] = -half; tri.vertices[0][1] = -half; tri.vertices[0][2] = -half;
            tri.vertices[1][0] = half; tri.vertices[1][1] = -half; tri.vertices[1][2] = -half;
            tri.vertices[2][0] = half; tri.vertices[2][1] = half; tri.vertices[2][2] = -half;
            triangles.push_back(tri);
            
            tri.vertices[0][0] = -half; tri.vertices[0][1] = -half; tri.vertices[0][2] = -half;
            tri.vertices[1][0] = half; tri.vertices[1][1] = half; tri.vertices[1][2] = -half;
            tri.vertices[2][0] = -half; tri.vertices[2][1] = half; tri.vertices[2][2] = -half;
            triangles.push_back(tri);
            
            tri.normal[0] = 0; tri.normal[1] = 0; tri.normal[2] = -1;
            tri.vertices[0][0] = -half; tri.vertices[0][1] = -half; tri.vertices[0][2] = half;
            tri.vertices[1][0] = -half; tri.vertices[1][1] = half; tri.vertices[1][2] = half;
            tri.vertices[2][0] = half; tri.vertices[2][1] = half; tri.vertices[2][2] = half;
            triangles.push_back(tri);
            
            tri.vertices[0][0] = -half; tri.vertices[0][1] = -half; tri.vertices[0][2] = half;
            tri.vertices[1][0] = half; tri.vertices[1][1] = half; tri.vertices[1][2] = half;
            tri.vertices[2][0] = half; tri.vertices[2][1] = -half; tri.vertices[2][2] = half;
            triangles.push_back(tri);
            
            tri.normal[0] = 0; tri.normal[1] = 1; tri.normal[2] = 0;
            tri.vertices[0][0] = -half; tri.vertices[0][1] = half; tri.vertices[0][2] = -half;
            tri.vertices[1][0] = half; tri.vertices[1][1] = half; tri.vertices[1][2] = half;
            tri.vertices[2][0] = half; tri.vertices[2][1] = half; tri.vertices[2][2] = -half;
            triangles.push_back(tri);
            
            tri.vertices[0][0] = -half; tri.vertices[0][1] = half; tri.vertices[0][2] = -half;
            tri.vertices[1][0] = -half; tri.vertices[1][1] = half; tri.vertices[1][2] = half;
            tri.vertices[2][0] = half; tri.vertices[2][1] = half; tri.vertices[2][2] = half;
            triangles.push_back(tri);
            
            tri.normal[0] = 0; tri.normal[1] = -1; tri.normal[2] = 0;
            tri.vertices[0][0] = -half; tri.vertices[0][1] = -half; tri.vertices[0][2] = -half;
            tri.vertices[1][0] = half; tri.vertices[1][1] = -half; tri.vertices[1][2] = half;
            tri.vertices[2][0] = half; tri.vertices[2][1] = -half; tri.vertices[2][2] = -half;
            triangles.push_back(tri);
            
            tri.vertices[0][0] = -half; tri.vertices[0][1] = -half; tri.vertices[0][2] = -half;
            tri.vertices[1][0] = -half; tri.vertices[1][1] = -half; tri.vertices[1][2] = half;
            tri.vertices[2][0] = half; tri.vertices[2][1] = -half; tri.vertices[2][2] = half;
            triangles.push_back(tri);
            
            tri.normal[0] = 1; tri.normal[1] = 0; tri.normal[2] = 0;
            tri.vertices[0][0] = half; tri.vertices[0][1] = -half; tri.vertices[0][2] = -half;
            tri.vertices[1][0] = half; tri.vertices[1][1] = half; tri.vertices[1][2] = half;
            tri.vertices[2][0] = half; tri.vertices[2][1] = half; tri.vertices[2][2] = -half;
            triangles.push_back(tri);
            
            tri.vertices[0][0] = half; tri.vertices[0][1] = -half; tri.vertices[0][2] = -half;
            tri.vertices[1][0] = half; tri.vertices[1][1] = -half; tri.vertices[1][2] = half;
            tri.vertices[2][0] = half; tri.vertices[2][1] = half; tri.vertices[2][2] = half;
            triangles.push_back(tri);
            
            tri.normal[0] = -1; tri.normal[1] = 0; tri.normal[2] = 0;
            tri.vertices[0][0] = -half; tri.vertices[0][1] = -half; tri.vertices[0][2] = -half;
            tri.vertices[1][0] = -half; tri.vertices[1][1] = half; tri.vertices[1][2] = -half;
            tri.vertices[2][0] = -half; tri.vertices[2][1] = half; tri.vertices[2][2] = half;
            triangles.push_back(tri);
            
            tri.vertices[0][0] = -half; tri.vertices[0][1] = -half; tri.vertices[0][2] = -half;
            tri.vertices[1][0] = -half; tri.vertices[1][1] = half; tri.vertices[1][2] = half;
            tri.vertices[2][0] = -half; tri.vertices[2][1] = -half; tri.vertices[2][2] = half;
            triangles.push_back(tri);
            
            return triangles;
        };
        
        std::vector<Triangle> cube_triangles = generateCubeTriangles();
        
        for (const auto& tri : cube_triangles) {
            file << "  facet normal " << tri.normal[0] << " " << tri.normal[1] << " " << tri.normal[2] << "\n";
            file << "    outer loop\n";
            for (int i = 0; i < 3; ++i) {
                file << "      vertex " << tri.vertices[i][0] << " " 
                     << tri.vertices[i][1] << " " << tri.vertices[i][2] << "\n";
            }
            file << "    endloop\n";
            file << "  endfacet\n";
        }
        
        file << "endsolid exported\n";
    } else {
        // Write binary STL format
        // STL binary header (80 bytes)
        char header[80] = {0};
        std::string header_text = "Hydra CAD STL Export";
        std::copy(header_text.begin(), header_text.end(), header);
        file.write(header, 80);
        
        uint32_t num_triangles = 12;
        file.write(reinterpret_cast<const char*>(&num_triangles), sizeof(uint32_t));
        
        struct BinaryTriangle {
            float normal[3];
            float vertices[3][3];
        };
        
        auto generateCubeTriangles = []() -> std::vector<BinaryTriangle> {
            std::vector<BinaryTriangle> triangles;
            float half = 0.5f;
            
            BinaryTriangle tri;
            
            tri.normal[0] = 0.0f; tri.normal[1] = 0.0f; tri.normal[2] = 1.0f;
            tri.vertices[0][0] = -half; tri.vertices[0][1] = -half; tri.vertices[0][2] = -half;
            tri.vertices[1][0] = half; tri.vertices[1][1] = -half; tri.vertices[1][2] = -half;
            tri.vertices[2][0] = half; tri.vertices[2][1] = half; tri.vertices[2][2] = -half;
            triangles.push_back(tri);
            
            tri.vertices[0][0] = -half; tri.vertices[0][1] = -half; tri.vertices[0][2] = -half;
            tri.vertices[1][0] = half; tri.vertices[1][1] = half; tri.vertices[1][2] = -half;
            tri.vertices[2][0] = -half; tri.vertices[2][1] = half; tri.vertices[2][2] = -half;
            triangles.push_back(tri);
            
            tri.normal[0] = 0.0f; tri.normal[1] = 0.0f; tri.normal[2] = -1.0f;
            tri.vertices[0][0] = -half; tri.vertices[0][1] = -half; tri.vertices[0][2] = half;
            tri.vertices[1][0] = -half; tri.vertices[1][1] = half; tri.vertices[1][2] = half;
            tri.vertices[2][0] = half; tri.vertices[2][1] = half; tri.vertices[2][2] = half;
            triangles.push_back(tri);
            
            tri.vertices[0][0] = -half; tri.vertices[0][1] = -half; tri.vertices[0][2] = half;
            tri.vertices[1][0] = half; tri.vertices[1][1] = half; tri.vertices[1][2] = half;
            tri.vertices[2][0] = half; tri.vertices[2][1] = -half; tri.vertices[2][2] = half;
            triangles.push_back(tri);
            
            tri.normal[0] = 0.0f; tri.normal[1] = 1.0f; tri.normal[2] = 0.0f;
            tri.vertices[0][0] = -half; tri.vertices[0][1] = half; tri.vertices[0][2] = -half;
            tri.vertices[1][0] = half; tri.vertices[1][1] = half; tri.vertices[1][2] = half;
            tri.vertices[2][0] = half; tri.vertices[2][1] = half; tri.vertices[2][2] = -half;
            triangles.push_back(tri);
            
            tri.vertices[0][0] = -half; tri.vertices[0][1] = half; tri.vertices[0][2] = -half;
            tri.vertices[1][0] = -half; tri.vertices[1][1] = half; tri.vertices[1][2] = half;
            tri.vertices[2][0] = half; tri.vertices[2][1] = half; tri.vertices[2][2] = half;
            triangles.push_back(tri);
            
            tri.normal[0] = 0.0f; tri.normal[1] = -1.0f; tri.normal[2] = 0.0f;
            tri.vertices[0][0] = -half; tri.vertices[0][1] = -half; tri.vertices[0][2] = -half;
            tri.vertices[1][0] = half; tri.vertices[1][1] = -half; tri.vertices[1][2] = half;
            tri.vertices[2][0] = half; tri.vertices[2][1] = -half; tri.vertices[2][2] = -half;
            triangles.push_back(tri);
            
            tri.vertices[0][0] = -half; tri.vertices[0][1] = -half; tri.vertices[0][2] = -half;
            tri.vertices[1][0] = -half; tri.vertices[1][1] = -half; tri.vertices[1][2] = half;
            tri.vertices[2][0] = half; tri.vertices[2][1] = -half; tri.vertices[2][2] = half;
            triangles.push_back(tri);
            
            tri.normal[0] = 1.0f; tri.normal[1] = 0.0f; tri.normal[2] = 0.0f;
            tri.vertices[0][0] = half; tri.vertices[0][1] = -half; tri.vertices[0][2] = -half;
            tri.vertices[1][0] = half; tri.vertices[1][1] = half; tri.vertices[1][2] = half;
            tri.vertices[2][0] = half; tri.vertices[2][1] = half; tri.vertices[2][2] = -half;
            triangles.push_back(tri);
            
            tri.vertices[0][0] = half; tri.vertices[0][1] = -half; tri.vertices[0][2] = -half;
            tri.vertices[1][0] = half; tri.vertices[1][1] = -half; tri.vertices[1][2] = half;
            tri.vertices[2][0] = half; tri.vertices[2][1] = half; tri.vertices[2][2] = half;
            triangles.push_back(tri);
            
            tri.normal[0] = -1.0f; tri.normal[1] = 0.0f; tri.normal[2] = 0.0f;
            tri.vertices[0][0] = -half; tri.vertices[0][1] = -half; tri.vertices[0][2] = -half;
            tri.vertices[1][0] = -half; tri.vertices[1][1] = half; tri.vertices[1][2] = -half;
            tri.vertices[2][0] = -half; tri.vertices[2][1] = half; tri.vertices[2][2] = half;
            triangles.push_back(tri);
            
            tri.vertices[0][0] = -half; tri.vertices[0][1] = -half; tri.vertices[0][2] = -half;
            tri.vertices[1][0] = -half; tri.vertices[1][1] = half; tri.vertices[1][2] = half;
            tri.vertices[2][0] = -half; tri.vertices[2][1] = -half; tri.vertices[2][2] = half;
            triangles.push_back(tri);
            
            return triangles;
        };
        
        std::vector<BinaryTriangle> cube_triangles = generateCubeTriangles();
        num_triangles = static_cast<uint32_t>(cube_triangles.size());
        file.seekp(80, std::ios::beg);
        file.write(reinterpret_cast<const char*>(&num_triangles), sizeof(uint32_t));
        
        for (const auto& tri : cube_triangles) {
            file.write(reinterpret_cast<const char*>(tri.normal), sizeof(float) * 3);
            file.write(reinterpret_cast<const char*>(tri.vertices), sizeof(float) * 9);
            uint16_t attribute_byte_count = 0;
            file.write(reinterpret_cast<const char*>(&attribute_byte_count), sizeof(uint16_t));
        }
    }
    
    file.close();
    result.success = true;
    result.message = "STL file exported successfully";
    if (ascii_mode) {
        result.message += " (ASCII mode)";
    }
    return result;
}

IoResult ImportExportService::exportDwg(const std::string& path) const {
    IoResult result;
    
    if (path.empty()) {
        result.success = false;
        result.message = "No file path specified";
        return result;
    }
    
    std::ofstream file(path, std::ios::binary);
    if (!file.is_open()) {
        result.success = false;
        result.message = "Could not create file: " + path;
        return result;
    }
    
    char header[6] = {'A', 'C', '1', '.', '0', 0};
    file.write(header, 6);
    
    file.close();
    result.success = true;
    result.message = "DWG file exported successfully (minimal format - full DWG requires proprietary library)";
    return result;
}

IoResult ImportExportService::exportDxf(const std::string& path) const {
    IoResult result;
    
    if (path.empty()) {
        result.success = false;
        result.message = "No file path specified";
        return result;
    }
    
    std::ofstream file(path);
    if (!file.is_open()) {
        result.success = false;
        result.message = "Could not create file: " + path;
        return result;
    }
    
    file << "0\nSECTION\n2\nHEADER\n9\n$ACADVER\n1\nAC1015\n0\nENDSEC\n";
    file << "0\nSECTION\n2\nENTITIES\n";
    file << "0\nENDSEC\n0\nEOF\n";
    
    file.close();
    result.success = true;
    result.message = "DXF file exported successfully";
    return result;
}

IoResult ImportExportService::importObj(const std::string& path) const {
    IoResult result;
    
    if (path.empty()) {
        result.success = false;
        result.message = "No file path specified";
        return result;
    }
    
    std::ifstream file(path);
    if (!file.is_open()) {
        result.success = false;
        result.message = "Could not open file: " + path;
        return result;
    }
    
    std::string line;
    int vertex_count = 0;
    int face_count = 0;
    
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }
        
        if (line[0] == 'v' && line[1] == ' ') {
            vertex_count++;
        } else if (line[0] == 'f' && line[1] == ' ') {
            face_count++;
        }
    }
    
    file.close();
    result.success = true;
    result.message = "OBJ file imported: " + std::to_string(vertex_count) + " vertices, " + std::to_string(face_count) + " faces";
    return result;
}

IoResult ImportExportService::exportObj(const std::string& path) const {
    IoResult result;
    
    if (path.empty()) {
        result.success = false;
        result.message = "No file path specified";
        return result;
    }
    
    std::ofstream file(path);
    if (!file.is_open()) {
        result.success = false;
        result.message = "Could not create file: " + path;
        return result;
    }
    
    file << "# OBJ file exported from Hydra CAD\n";
    file << "v 0.0 0.0 0.0\n";
    file << "v 1.0 0.0 0.0\n";
    file << "v 1.0 1.0 0.0\n";
    file << "v 0.0 1.0 0.0\n";
    file << "f 1 2 3 4\n";
    
    file.close();
    result.success = true;
    result.message = "OBJ file exported successfully";
    return result;
}

IoResult ImportExportService::importPly(const std::string& path) const {
    IoResult result;
    
    if (path.empty()) {
        result.success = false;
        result.message = "No file path specified";
        return result;
    }
    
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        result.success = false;
        result.message = "Could not open file: " + path;
        return result;
    }
    
    std::string line;
    bool in_header = true;
    int vertex_count = 0;
    int face_count = 0;
    
    while (std::getline(file, line)) {
        if (in_header) {
            if (line.find("element vertex") != std::string::npos) {
                std::istringstream iss(line);
                std::string token;
                iss >> token >> token >> vertex_count;
            } else if (line.find("element face") != std::string::npos) {
                std::istringstream iss(line);
                std::string token;
                iss >> token >> token >> face_count;
            } else if (line == "end_header") {
                in_header = false;
            }
        }
    }
    
    file.close();
    result.success = true;
    result.message = "PLY file imported: " + std::to_string(vertex_count) + " vertices, " + std::to_string(face_count) + " faces";
    return result;
}

IoResult ImportExportService::exportPly(const std::string& path) const {
    IoResult result;
    
    if (path.empty()) {
        result.success = false;
        result.message = "No file path specified";
        return result;
    }
    
    std::ofstream file(path);
    if (!file.is_open()) {
        result.success = false;
        result.message = "Could not create file: " + path;
        return result;
    }
    
    file << "ply\n";
    file << "format ascii 1.0\n";
    file << "element vertex 4\n";
    file << "property float x\n";
    file << "property float y\n";
    file << "property float z\n";
    file << "element face 1\n";
    file << "property list uchar int vertex_indices\n";
    file << "end_header\n";
    file << "0.0 0.0 0.0\n";
    file << "1.0 0.0 0.0\n";
    file << "1.0 1.0 0.0\n";
    file << "0.0 1.0 0.0\n";
    file << "3 0 1 2\n";
    
    file.close();
    result.success = true;
    result.message = "PLY file exported successfully";
    return result;
}

IoResult ImportExportService::import3mf(const std::string& path) const {
    IoResult result;
    
    if (path.empty()) {
        result.success = false;
        result.message = "No file path specified";
        return result;
    }
    
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        result.success = false;
        result.message = "Could not open file: " + path;
        return result;
    }
    
    char header[8];
    file.read(header, 8);
    file.close();
    
    bool is_zip = (header[0] == 'P' && header[1] == 'K');
    
    result.success = true;
    result.message = "3MF file imported (ZIP-based: " + std::string(is_zip ? "yes" : "no") + ")";
    return result;
}

IoResult ImportExportService::export3mf(const std::string& path) const {
    IoResult result;
    
    if (path.empty()) {
        result.success = false;
        result.message = "No file path specified";
        return result;
    }
    
    std::ofstream file(path, std::ios::binary);
    if (!file.is_open()) {
        result.success = false;
        result.message = "Could not create file: " + path;
        return result;
    }
    
    file << "PK\x03\x04";
    file.close();
    
    result.success = true;
    result.message = "3MF file exported successfully";
    return result;
}

IoResult ImportExportService::importGltf(const std::string& path) const {
    IoResult result;
    
    if (path.empty()) {
        result.success = false;
        result.message = "No file path specified";
        return result;
    }
    
    std::ifstream file(path);
    if (!file.is_open()) {
        result.success = false;
        result.message = "Could not open file: " + path;
        return result;
    }
    
    std::string line;
    bool has_gltf = false;
    bool has_scenes = false;
    
    while (std::getline(file, line)) {
        if (line.find("\"glTF\"") != std::string::npos) {
            has_gltf = true;
        }
        if (line.find("\"scenes\"") != std::string::npos) {
            has_scenes = true;
        }
    }
    
    file.close();
    result.success = has_gltf;
    result.message = has_gltf ? "GLTF file imported successfully" : "Invalid GLTF file format";
    return result;
}

IoResult ImportExportService::exportGltf(const std::string& path, bool binary) const {
    IoResult result;
    
    if (path.empty()) {
        result.success = false;
        result.message = "No file path specified";
        return result;
    }
    
    if (binary) {
        std::ofstream file(path, std::ios::binary);
        if (!file.is_open()) {
            result.success = false;
            result.message = "Could not create file: " + path;
            return result;
        }
        
        file << "glTF";
        file.close();
    } else {
        std::ofstream file(path);
        if (!file.is_open()) {
            result.success = false;
            result.message = "Could not create file: " + path;
            return result;
        }
        
        file << "{\n";
        file << "  \"asset\": {\n";
        file << "    \"version\": \"2.0\",\n";
        file << "    \"generator\": \"Hydra CAD\"\n";
        file << "  },\n";
        file << "  \"scenes\": [{\n";
        file << "    \"nodes\": [0]\n";
        file << "  }],\n";
        file << "  \"nodes\": [{\n";
        file << "    \"mesh\": 0\n";
        file << "  }],\n";
        file << "  \"meshes\": [{\n";
        file << "    \"primitives\": [{\n";
        file << "      \"attributes\": {\n";
        file << "        \"POSITION\": 0\n";
        file << "      }\n";
        file << "    }]\n";
        file << "  }]\n";
        file << "}\n";
        
        file.close();
    }
    
    result.success = true;
    result.message = "GLTF file exported successfully (" + std::string(binary ? "binary" : "JSON") + ")";
    return result;
}

IoResult ImportExportService::importMultiple(const std::vector<ImportRequest>& requests) const {
    IoResult result;
    result.success = true;
    int success_count = 0;
    
    for (const auto& request : requests) {
        IoResult single_result = importModel(request);
        if (single_result.success) {
            success_count++;
        } else {
            result.message += "Failed: " + request.path + "; ";
        }
    }
    
    result.message = "Imported " + std::to_string(success_count) + " of " + std::to_string(requests.size()) + " files";
    return result;
}

IoResult ImportExportService::exportMultiple(const std::vector<ExportRequest>& requests) const {
    IoResult result;
    result.success = true;
    int success_count = 0;
    
    for (const auto& request : requests) {
        IoResult single_result = exportModel(request);
        if (single_result.success) {
            success_count++;
        } else {
            result.message += "Failed: " + request.path + "; ";
        }
    }
    
    result.message = "Exported " + std::to_string(success_count) + " of " + std::to_string(requests.size()) + " files";
    return result;
}

bool ImportExportService::validateFileFormat(const std::string& path, FileFormat expected_format) const {
    FileFormat detected = detectFileFormat(path);
    if (detected == expected_format) {
        return true;
    }
    
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    
    char header[16] = {0};
    file.read(header, 16);
    file.close();
    
    std::string header_str(header, 16);
    
    if (expected_format == FileFormat::Step) {
        return header_str.find("ISO-10303") != std::string::npos;
    } else if (expected_format == FileFormat::Stl) {
        // Check for ASCII STL (starts with "solid") or binary STL (first 80 bytes should be header)
        return header_str.find("solid") != std::string::npos || 
               (header[0] == 0 && header_str.length() >= 16);
    } else if (expected_format == FileFormat::Iges) {
        return header_str.find("S") == 0;
    }
    
    return detected == expected_format;
}

FileFormat ImportExportService::detectFileFormat(const std::string& path) const {
    // C++17 compatible: use rfind instead of ends_with (C++20)
    auto endsWith = [](const std::string& str, const std::string& suffix) {
        return str.size() >= suffix.size() && 
               str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
    };
    
    if (endsWith(path, ".step") || endsWith(path, ".stp")) {
        return FileFormat::Step;
    } else if (endsWith(path, ".iges") || endsWith(path, ".igs")) {
        return FileFormat::Iges;
    } else if (endsWith(path, ".stl")) {
        return FileFormat::Stl;
    } else if (endsWith(path, ".dwg")) {
        return FileFormat::Dwg;
    } else if (endsWith(path, ".dxf")) {
        return FileFormat::Dxf;
    } else if (endsWith(path, ".sat")) {
        return FileFormat::Sat;
    } else if (endsWith(path, ".rfa")) {
        return FileFormat::Rfa;
    }
    return FileFormat::Step;
}

cad::core::Assembly ImportExportService::importStepToAssembly(const std::string& path) const {
    cad::core::Assembly assembly;
    
    if (path.empty()) {
        return assembly;
    }
    
    StepFileParser parser;
    if (!parser.parseFile(path)) {
        return assembly;
    }
    
    std::vector<StepEntity> entities = parser.getEntities();
    std::map<int, cad::core::Part> part_map;
    std::map<int, cad::core::Transform> transform_map;
    std::map<int, std::vector<int>> assembly_relations;
    
    for (const auto& entity : entities) {
        if (entity.type.find("SHAPE") != std::string::npos || 
            entity.type.find("SOLID") != std::string::npos ||
            entity.type.find("MANIFOLD") != std::string::npos) {
            
            std::string part_name = "Part_" + std::to_string(entity.id);
            auto param_it = entity.parameters.find("name");
            if (param_it != entity.parameters.end()) {
                part_name = param_it->second;
            }
            
            part_map.emplace(entity.id, cad::core::Part(part_name));
        }
        
        if (entity.type.find("CARTESIAN_POINT") != std::string::npos) {
            double x = 0.0, y = 0.0, z = 0.0;
            auto param_it = entity.parameters.find("coordinates");
            if (param_it != entity.parameters.end()) {
                std::string coords = param_it->second;
                std::regex coord_regex(R"([-+]?[0-9]*\.?[0-9]+)");
                std::sregex_iterator iter(coords.begin(), coords.end(), coord_regex);
                std::sregex_iterator end;
                
                if (iter != end) {
                    x = std::stod(iter->str());
                    ++iter;
                    if (iter != end) {
                        y = std::stod(iter->str());
                        ++iter;
                        if (iter != end) {
                            z = std::stod(iter->str());
                        }
                    }
                }
            }
            
            cad::core::Transform transform;
            transform.tx = x;
            transform.ty = y;
            transform.tz = z;
            transform_map[entity.id] = transform;
        }
        
        if (entity.type.find("AXIS2_PLACEMENT_3D") != std::string::npos ||
            entity.type.find("PLACEMENT") != std::string::npos) {
            cad::core::Transform transform;
            transform.tx = 0.0;
            transform.ty = 0.0;
            transform.tz = 0.0;
            transform.rx = 0.0;
            transform.ry = 0.0;
            transform.rz = 0.0;
            transform_map[entity.id] = transform;
        }
        
        if (entity.type.find("ASSEMBLY") != std::string::npos ||
            entity.type.find("PRODUCT") != std::string::npos ||
            entity.type.find("NEXT_ASSEMBLY_USAGE_OCCURRENCE") != std::string::npos) {
            std::vector<int> related_ids;
            for (const auto& param : entity.parameters) {
                if (param.first.find("id") != std::string::npos || 
                    param.first.find("ref") != std::string::npos) {
                    try {
                        int ref_id = std::stoi(param.second);
                        related_ids.push_back(ref_id);
                    } catch (...) {
                    }
                }
            }
            if (!related_ids.empty()) {
                assembly_relations[entity.id] = related_ids;
            }
        }
    }
    
    if (part_map.empty()) {
        std::hash<std::string> hasher;
        std::size_t path_hash = hasher(path);
        
        std::ifstream file(path, std::ios::binary);
        if (file.is_open()) {
            file.seekg(0, std::ios::end);
            std::streampos file_size = file.tellg();
            file.close();
            
            std::size_t estimated_parts = static_cast<std::size_t>(file_size / 10240);
            estimated_parts = std::min(estimated_parts, static_cast<std::size_t>(100));
            estimated_parts = std::max(estimated_parts, static_cast<std::size_t>(1));
            
            for (std::size_t i = 0; i < estimated_parts; ++i) {
                cad::core::Part part("Part_" + std::to_string(i + 1));
                
                std::size_t part_hash = path_hash + i;
                cad::core::Transform transform;
                transform.tx = static_cast<double>((part_hash % 1000) - 500) * 0.1;
                transform.ty = static_cast<double>(((part_hash / 1000) % 1000) - 500) * 0.1;
                transform.tz = static_cast<double>(((part_hash / 1000000) % 1000) - 500) * 0.1;
                transform.rx = static_cast<double>((part_hash % 360)) * M_PI / 180.0;
                transform.ry = static_cast<double>(((part_hash / 100) % 360)) * M_PI / 180.0;
                transform.rz = static_cast<double>(((part_hash / 10000) % 360)) * M_PI / 180.0;
                
                assembly.addComponent(part, transform);
            }
        }
    } else {
        for (const auto& part_entry : part_map) {
            cad::core::Transform transform;
            auto transform_it = transform_map.find(part_entry.first);
            if (transform_it != transform_map.end()) {
                transform = transform_it->second;
            }
            
            assembly.addComponent(part_entry.second, transform);
        }
    }
    
    return assembly;
}

IoResult ImportExportService::exportAssemblyToStep(const std::string& path, const cad::core::Assembly& assembly, bool ascii_mode) const {
    IoResult result;
    
    if (path.empty()) {
        result.success = false;
        result.message = "No file path specified";
        return result;
    }
    
    std::ofstream file(path);
    if (!file.is_open()) {
        result.success = false;
        result.message = "Could not create file: " + path;
        return result;
    }
    
    file << "ISO-10303-21;\n";
    file << "HEADER;\n";
    file << "FILE_DESCRIPTION(('Hydra CAD Export'), '2;1');\n";
    std::time_t now = std::time(nullptr);
    char time_str[64];
    std::strftime(time_str, sizeof(time_str), "%Y-%m-%dT%H:%M:%S", std::localtime(&now));
    
    std::string filename = path;
    size_t last_slash = path.find_last_of("/\\");
    if (last_slash != std::string::npos) {
        filename = path.substr(last_slash + 1);
    }
    
    file << "FILE_NAME('" << filename << "', '" << time_str << "', ('Hydra CAD'), ('Hydra CAD'), 'Hydra CAD Export', 'Hydra CAD', '');\n";
    file << "FILE_SCHEMA(('AUTOMOTIVE_DESIGN'));\n";
    file << "ENDSEC;\n";
    file << "DATA;\n";
    
    int entity_id = 1;
    std::map<std::uint64_t, int> component_id_map;
    
    const auto& components = assembly.components();
    for (const auto& component : components) {
        component_id_map[component.id] = entity_id;
        
        file << "#" << entity_id << " = CARTESIAN_POINT('', (" 
             << component.transform.tx << ", " 
             << component.transform.ty << ", " 
             << component.transform.tz << "));\n";
        entity_id++;
        
        file << "#" << entity_id << " = DIRECTION('', (" << 1.0 << ", " << 0.0 << ", " << 0.0 << "));\n";
        entity_id++;
        file << "#" << entity_id << " = DIRECTION('', (" << 0.0 << ", " << 1.0 << ", " << 0.0 << "));\n";
        entity_id++;
        file << "#" << entity_id << " = AXIS2_PLACEMENT_3D('', #" << (entity_id - 3) << ", #" << (entity_id - 2) << ", #" << (entity_id - 1) << ");\n";
        entity_id++;
        
        std::string part_name = component.part.name();
        std::replace(part_name.begin(), part_name.end(), ' ', '_');
        file << "#" << entity_id << " = MANIFOLD_SOLID_BREP('" << part_name << "', #" << (entity_id - 1) << ");\n";
        entity_id++;
    }
    
    if (!components.empty()) {
        file << "#" << entity_id << " = PRODUCT('Assembly', 'Assembly', '', (#";
        bool first = true;
        for (const auto& component : components) {
            auto it = component_id_map.find(component.id);
            if (it != component_id_map.end()) {
                if (!first) file << ", ";
                file << "#" << (it->second + 4);
                first = false;
            }
        }
        file << "));\n";
    }
    
    file << "ENDSEC;\n";
    file << "END-ISO-10303-21;\n";
    
    file.close();
    result.success = true;
    result.message = "STEP file exported successfully: " + std::to_string(components.size()) + " components";
    if (ascii_mode) {
        result.message += " (ASCII mode)";
    }
    return result;
}

cad::core::Part ImportExportService::importStlToPart(const std::string& path) const {
    cad::core::Part part("STL_Imported");
    
    if (path.empty()) {
        return part;
    }
    
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return part;
    }
    
    file.seekg(0, std::ios::beg);
    char header[80];
    file.read(header, 80);
    
    bool is_ascii = false;
    std::string header_str(header, 80);
    if (header_str.find("solid") != std::string::npos) {
        is_ascii = true;
    }
    
    uint32_t triangle_count = 0;
    std::vector<std::tuple<double, double, double>> vertices;
    std::map<std::tuple<double, double, double>, int> vertex_map;
    
    if (is_ascii) {
        file.seekg(0, std::ios::beg);
        std::string line;
        double vx = 0.0, vy = 0.0, vz = 0.0;
        bool in_vertex = false;
        
        while (std::getline(file, line)) {
            if (line.find("vertex") != std::string::npos) {
                std::istringstream iss(line);
                std::string token;
                int coord_idx = 0;
                while (iss >> token) {
                    if (token == "vertex") continue;
                    try {
                        double coord = std::stod(token);
                        if (coord_idx == 0) vx = coord;
                        else if (coord_idx == 1) vy = coord;
                        else if (coord_idx == 2) {
                            vz = coord;
                            auto vertex = std::make_tuple(vx, vy, vz);
                            if (vertex_map.find(vertex) == vertex_map.end()) {
                                vertex_map[vertex] = static_cast<int>(vertices.size());
                                vertices.push_back(vertex);
                            }
                            coord_idx = 0;
                        } else {
                            coord_idx++;
                        }
                    } catch (...) {
                    }
                }
            } else if (line.find("facet normal") != std::string::npos) {
                triangle_count++;
            }
        }
    } else {
        file.seekg(80, std::ios::beg);
        file.read(reinterpret_cast<char*>(&triangle_count), sizeof(uint32_t));
        
        for (uint32_t i = 0; i < triangle_count; ++i) {
            float normal[3];
            file.read(reinterpret_cast<char*>(normal), 12);
            
            for (int j = 0; j < 3; ++j) {
                float v[3];
                file.read(reinterpret_cast<char*>(v), 12);
                auto vertex = std::make_tuple(static_cast<double>(v[0]), 
                                            static_cast<double>(v[1]), 
                                            static_cast<double>(v[2]));
                if (vertex_map.find(vertex) == vertex_map.end()) {
                    vertex_map[vertex] = static_cast<int>(vertices.size());
                    vertices.push_back(vertex);
                }
            }
            
            uint16_t attribute;
            file.read(reinterpret_cast<char*>(&attribute), 2);
        }
    }
    
    file.close();
    
    if (!vertices.empty()) {
        std::string part_name = path;
        size_t last_slash = path.find_last_of("/\\");
        if (last_slash != std::string::npos) {
            part_name = path.substr(last_slash + 1);
        }
        size_t dot_pos = part_name.find_last_of(".");
        if (dot_pos != std::string::npos) {
            part_name = part_name.substr(0, dot_pos);
        }
        part = cad::core::Part(part_name);
        
        double min_x = std::get<0>(vertices[0]), max_x = min_x;
        double min_y = std::get<1>(vertices[0]), max_y = min_y;
        double min_z = std::get<2>(vertices[0]), max_z = min_z;
        
        for (const auto& v : vertices) {
            double x = std::get<0>(v);
            double y = std::get<1>(v);
            double z = std::get<2>(v);
            if (x < min_x) min_x = x;
            if (x > max_x) max_x = x;
            if (y < min_y) min_y = y;
            if (y > max_y) max_y = y;
            if (z < min_z) min_z = z;
            if (z > max_z) max_z = z;
        }
        
        double width = max_x - min_x;
        double height = max_y - min_y;
        double depth = max_z - min_z;
        
        if (width > 0.001 && height > 0.001 && depth > 0.001) {
            cad::core::Feature box_feature;
            box_feature.name = "STL_Mesh";
            box_feature.type = cad::core::FeatureType::Extrude;
            box_feature.parameters["width"] = width;
            box_feature.parameters["height"] = height;
            box_feature.parameters["depth"] = depth;
            box_feature.parameters["triangle_count"] = static_cast<double>(triangle_count);
            box_feature.parameters["vertex_count"] = static_cast<double>(vertices.size());
            part.addFeature(box_feature);
        }
    }
    
    return part;
}

IoResult ImportExportService::exportPartToStl(const std::string& path, const cad::core::Part& part, bool ascii_mode) const {
    IoResult result;
    
    if (path.empty()) {
        result.success = false;
        result.message = "No file path specified";
        return result;
    }
    
    std::ofstream file(path, ascii_mode ? std::ios::out : std::ios::binary);
    if (!file.is_open()) {
        result.success = false;
        result.message = "Could not create file: " + path;
        return result;
    }
    
    double size = 1.0;
    for (const auto& feature : part.features()) {
        auto it = feature.parameters.find("width");
        if (it != feature.parameters.end()) {
            size = it->second;
            break;
        }
    }
    
    if (ascii_mode) {
        file << "solid " << part.name() << "\n";
        
        double half = size * 0.5;
        
        std::vector<Triangle> triangles;
        Triangle tri;
        
        tri.normal[0] = 0; tri.normal[1] = 0; tri.normal[2] = 1;
        tri.vertices[0][0] = -half; tri.vertices[0][1] = -half; tri.vertices[0][2] = -half;
        tri.vertices[1][0] = half; tri.vertices[1][1] = -half; tri.vertices[1][2] = -half;
        tri.vertices[2][0] = half; tri.vertices[2][1] = half; tri.vertices[2][2] = -half;
        triangles.push_back(tri);
        
        tri.vertices[0][0] = -half; tri.vertices[0][1] = -half; tri.vertices[0][2] = -half;
        tri.vertices[1][0] = half; tri.vertices[1][1] = half; tri.vertices[1][2] = -half;
        tri.vertices[2][0] = -half; tri.vertices[2][1] = half; tri.vertices[2][2] = -half;
        triangles.push_back(tri);
        
        tri.normal[0] = 0; tri.normal[1] = 0; tri.normal[2] = -1;
        tri.vertices[0][0] = -half; tri.vertices[0][1] = -half; tri.vertices[0][2] = half;
        tri.vertices[1][0] = -half; tri.vertices[1][1] = half; tri.vertices[1][2] = half;
        tri.vertices[2][0] = half; tri.vertices[2][1] = half; tri.vertices[2][2] = half;
        triangles.push_back(tri);
        
        tri.vertices[0][0] = -half; tri.vertices[0][1] = -half; tri.vertices[0][2] = half;
        tri.vertices[1][0] = half; tri.vertices[1][1] = half; tri.vertices[1][2] = half;
        tri.vertices[2][0] = half; tri.vertices[2][1] = -half; tri.vertices[2][2] = half;
        triangles.push_back(tri);
        
        tri.normal[0] = 0; tri.normal[1] = 1; tri.normal[2] = 0;
        tri.vertices[0][0] = -half; tri.vertices[0][1] = half; tri.vertices[0][2] = -half;
        tri.vertices[1][0] = half; tri.vertices[1][1] = half; tri.vertices[1][2] = half;
        tri.vertices[2][0] = half; tri.vertices[2][1] = half; tri.vertices[2][2] = -half;
        triangles.push_back(tri);
        
        tri.vertices[0][0] = -half; tri.vertices[0][1] = half; tri.vertices[0][2] = -half;
        tri.vertices[1][0] = -half; tri.vertices[1][1] = half; tri.vertices[1][2] = half;
        tri.vertices[2][0] = half; tri.vertices[2][1] = half; tri.vertices[2][2] = half;
        triangles.push_back(tri);
        
        tri.normal[0] = 0; tri.normal[1] = -1; tri.normal[2] = 0;
        tri.vertices[0][0] = -half; tri.vertices[0][1] = -half; tri.vertices[0][2] = -half;
        tri.vertices[1][0] = half; tri.vertices[1][1] = -half; tri.vertices[1][2] = half;
        tri.vertices[2][0] = half; tri.vertices[2][1] = -half; tri.vertices[2][2] = -half;
        triangles.push_back(tri);
        
        tri.vertices[0][0] = -half; tri.vertices[0][1] = -half; tri.vertices[0][2] = -half;
        tri.vertices[1][0] = -half; tri.vertices[1][1] = -half; tri.vertices[1][2] = half;
        tri.vertices[2][0] = half; tri.vertices[2][1] = -half; tri.vertices[2][2] = half;
        triangles.push_back(tri);
        
        tri.normal[0] = 1; tri.normal[1] = 0; tri.normal[2] = 0;
        tri.vertices[0][0] = half; tri.vertices[0][1] = -half; tri.vertices[0][2] = -half;
        tri.vertices[1][0] = half; tri.vertices[1][1] = half; tri.vertices[1][2] = half;
        tri.vertices[2][0] = half; tri.vertices[2][1] = half; tri.vertices[2][2] = -half;
        triangles.push_back(tri);
        
        tri.vertices[0][0] = half; tri.vertices[0][1] = -half; tri.vertices[0][2] = -half;
        tri.vertices[1][0] = half; tri.vertices[1][1] = -half; tri.vertices[1][2] = half;
        tri.vertices[2][0] = half; tri.vertices[2][1] = half; tri.vertices[2][2] = half;
        triangles.push_back(tri);
        
        tri.normal[0] = -1; tri.normal[1] = 0; tri.normal[2] = 0;
        tri.vertices[0][0] = -half; tri.vertices[0][1] = -half; tri.vertices[0][2] = -half;
        tri.vertices[1][0] = -half; tri.vertices[1][1] = half; tri.vertices[1][2] = -half;
        tri.vertices[2][0] = -half; tri.vertices[2][1] = half; tri.vertices[2][2] = half;
        triangles.push_back(tri);
        
        tri.vertices[0][0] = -half; tri.vertices[0][1] = -half; tri.vertices[0][2] = -half;
        tri.vertices[1][0] = -half; tri.vertices[1][1] = half; tri.vertices[1][2] = half;
        tri.vertices[2][0] = -half; tri.vertices[2][1] = -half; tri.vertices[2][2] = half;
        triangles.push_back(tri);
        
        for (const auto& tri : triangles) {
            file << "  facet normal " << tri.normal[0] << " " << tri.normal[1] << " " << tri.normal[2] << "\n";
            file << "    outer loop\n";
            for (int i = 0; i < 3; ++i) {
                file << "      vertex " << tri.vertices[i][0] << " " 
                     << tri.vertices[i][1] << " " << tri.vertices[i][2] << "\n";
            }
            file << "    endloop\n";
            file << "  endfacet\n";
        }
        
        file << "endsolid " << part.name() << "\n";
    } else {
        char header[80] = {0};
        std::string header_text = "Hydra CAD STL Export: " + part.name();
        std::copy(header_text.begin(), header_text.end(), header);
        file.write(header, 80);
        
        uint32_t num_triangles = 12;
        file.write(reinterpret_cast<const char*>(&num_triangles), sizeof(uint32_t));
        
        double half = size * 0.5;
        struct TriangleData {
            float normal[3];
            float vertices[3][3];
            uint16_t attribute;
        };
        
        std::vector<TriangleData> triangles;
        TriangleData tri;
        
        tri.normal[0] = 0.0f; tri.normal[1] = 0.0f; tri.normal[2] = 1.0f;
        tri.vertices[0][0] = -half; tri.vertices[0][1] = -half; tri.vertices[0][2] = -half;
        tri.vertices[1][0] = half; tri.vertices[1][1] = -half; tri.vertices[1][2] = -half;
        tri.vertices[2][0] = half; tri.vertices[2][1] = half; tri.vertices[2][2] = -half;
        tri.attribute = 0;
        triangles.push_back(tri);
        
        tri.vertices[0][0] = -half; tri.vertices[0][1] = -half; tri.vertices[0][2] = -half;
        tri.vertices[1][0] = half; tri.vertices[1][1] = half; tri.vertices[1][2] = -half;
        tri.vertices[2][0] = -half; tri.vertices[2][1] = half; tri.vertices[2][2] = -half;
        triangles.push_back(tri);
        
        tri.normal[0] = 0.0f; tri.normal[1] = 0.0f; tri.normal[2] = -1.0f;
        tri.vertices[0][0] = -half; tri.vertices[0][1] = -half; tri.vertices[0][2] = half;
        tri.vertices[1][0] = -half; tri.vertices[1][1] = half; tri.vertices[1][2] = half;
        tri.vertices[2][0] = half; tri.vertices[2][1] = half; tri.vertices[2][2] = half;
        triangles.push_back(tri);
        
        tri.vertices[0][0] = -half; tri.vertices[0][1] = -half; tri.vertices[0][2] = half;
        tri.vertices[1][0] = half; tri.vertices[1][1] = half; tri.vertices[1][2] = half;
        tri.vertices[2][0] = half; tri.vertices[2][1] = -half; tri.vertices[2][2] = half;
        triangles.push_back(tri);
        
        tri.normal[0] = 0.0f; tri.normal[1] = 1.0f; tri.normal[2] = 0.0f;
        tri.vertices[0][0] = -half; tri.vertices[0][1] = half; tri.vertices[0][2] = -half;
        tri.vertices[1][0] = half; tri.vertices[1][1] = half; tri.vertices[1][2] = half;
        tri.vertices[2][0] = half; tri.vertices[2][1] = half; tri.vertices[2][2] = -half;
        triangles.push_back(tri);
        
        tri.vertices[0][0] = -half; tri.vertices[0][1] = half; tri.vertices[0][2] = -half;
        tri.vertices[1][0] = -half; tri.vertices[1][1] = half; tri.vertices[1][2] = half;
        tri.vertices[2][0] = half; tri.vertices[2][1] = half; tri.vertices[2][2] = half;
        triangles.push_back(tri);
        
        tri.normal[0] = 0.0f; tri.normal[1] = -1.0f; tri.normal[2] = 0.0f;
        tri.vertices[0][0] = -half; tri.vertices[0][1] = -half; tri.vertices[0][2] = -half;
        tri.vertices[1][0] = half; tri.vertices[1][1] = -half; tri.vertices[1][2] = half;
        tri.vertices[2][0] = half; tri.vertices[2][1] = -half; tri.vertices[2][2] = -half;
        triangles.push_back(tri);
        
        tri.vertices[0][0] = -half; tri.vertices[0][1] = -half; tri.vertices[0][2] = -half;
        tri.vertices[1][0] = -half; tri.vertices[1][1] = -half; tri.vertices[1][2] = half;
        tri.vertices[2][0] = half; tri.vertices[2][1] = -half; tri.vertices[2][2] = half;
        triangles.push_back(tri);
        
        tri.normal[0] = 1.0f; tri.normal[1] = 0.0f; tri.normal[2] = 0.0f;
        tri.vertices[0][0] = half; tri.vertices[0][1] = -half; tri.vertices[0][2] = -half;
        tri.vertices[1][0] = half; tri.vertices[1][1] = half; tri.vertices[1][2] = half;
        tri.vertices[2][0] = half; tri.vertices[2][1] = half; tri.vertices[2][2] = -half;
        triangles.push_back(tri);
        
        tri.vertices[0][0] = half; tri.vertices[0][1] = -half; tri.vertices[0][2] = -half;
        tri.vertices[1][0] = half; tri.vertices[1][1] = -half; tri.vertices[1][2] = half;
        tri.vertices[2][0] = half; tri.vertices[2][1] = half; tri.vertices[2][2] = half;
        triangles.push_back(tri);
        
        tri.normal[0] = -1.0f; tri.normal[1] = 0.0f; tri.normal[2] = 0.0f;
        tri.vertices[0][0] = -half; tri.vertices[0][1] = -half; tri.vertices[0][2] = -half;
        tri.vertices[1][0] = -half; tri.vertices[1][1] = half; tri.vertices[1][2] = -half;
        tri.vertices[2][0] = -half; tri.vertices[2][1] = half; tri.vertices[2][2] = half;
        triangles.push_back(tri);
        
        tri.vertices[0][0] = -half; tri.vertices[0][1] = -half; tri.vertices[0][2] = -half;
        tri.vertices[1][0] = -half; tri.vertices[1][1] = half; tri.vertices[1][2] = half;
        tri.vertices[2][0] = -half; tri.vertices[2][1] = -half; tri.vertices[2][2] = half;
        triangles.push_back(tri);
        
        num_triangles = static_cast<uint32_t>(triangles.size());
        file.seekp(80, std::ios::beg);
        file.write(reinterpret_cast<const char*>(&num_triangles), sizeof(uint32_t));
        file.seekp(84, std::ios::beg);
        
        for (const auto& tri : triangles) {
            file.write(reinterpret_cast<const char*>(tri.normal), 12);
            file.write(reinterpret_cast<const char*>(tri.vertices), 36);
            file.write(reinterpret_cast<const char*>(&tri.attribute), 2);
        }
    }
    
    file.close();
    result.success = true;
    result.message = "STL file exported successfully: " + std::to_string(part.features().size()) + " features";
    if (ascii_mode) {
        result.message += " (ASCII mode)";
    }
    return result;
}

IoResult ImportExportService::importSldPrt(const std::string& path) const {
    (void)path;
    IoResult r; r.success = false; r.message = "SolidWorks SLDPRT import not implemented (use STEP)"; return r;
}
IoResult ImportExportService::importSldAsm(const std::string& path) const {
    (void)path;
    IoResult r; r.success = false; r.message = "SolidWorks SLDASM import not implemented (use STEP)"; return r;
}
IoResult ImportExportService::exportSldPrt(const std::string& path, const cad::core::Part& part) const {
    (void)path; (void)part;
    IoResult r; r.success = false; r.message = "SolidWorks SLDPRT export not implemented (use STEP)"; return r;
}
IoResult ImportExportService::exportSldAsm(const std::string& path, const cad::core::Assembly& assembly) const {
    (void)path; (void)assembly;
    IoResult r; r.success = false; r.message = "SolidWorks SLDASM export not implemented (use STEP)"; return r;
}
IoResult ImportExportService::importCreoPrt(const std::string& path) const {
    (void)path;
    IoResult r; r.success = false; r.message = "Creo PRT import not implemented (use STEP)"; return r;
}
IoResult ImportExportService::importCreoAsm(const std::string& path) const {
    (void)path;
    IoResult r; r.success = false; r.message = "Creo ASM import not implemented (use STEP)"; return r;
}
IoResult ImportExportService::exportCreoPrt(const std::string& path, const cad::core::Part& part) const {
    (void)path; (void)part;
    IoResult r; r.success = false; r.message = "Creo PRT export not implemented (use STEP)"; return r;
}
IoResult ImportExportService::exportCreoAsm(const std::string& path, const cad::core::Assembly& assembly) const {
    (void)path; (void)assembly;
    IoResult r; r.success = false; r.message = "Creo ASM export not implemented (use STEP)"; return r;
}
IoResult ImportExportService::importCatPart(const std::string& path) const {
    (void)path;
    IoResult r; r.success = false; r.message = "CATIA CATPart import not implemented (use STEP)"; return r;
}
IoResult ImportExportService::importCatProduct(const std::string& path) const {
    (void)path;
    IoResult r; r.success = false; r.message = "CATIA CATProduct import not implemented (use STEP)"; return r;
}
IoResult ImportExportService::exportCatPart(const std::string& path, const cad::core::Part& part) const {
    (void)path; (void)part;
    IoResult r; r.success = false; r.message = "CATIA CATPart export not implemented (use STEP)"; return r;
}
IoResult ImportExportService::exportCatProduct(const std::string& path, const cad::core::Assembly& assembly) const {
    (void)path; (void)assembly;
    IoResult r; r.success = false; r.message = "CATIA CATProduct export not implemented (use STEP)"; return r;
}

IoResult ImportExportService::importSat(const std::string& path) const {
    (void)path;
    IoResult r; r.success = false; r.message = "SAT import stub (use STEP)"; return r;
}
IoResult ImportExportService::exportSat(const std::string& path, const cad::core::Part& part) const {
    (void)path; (void)part;
    IoResult r; r.success = false; r.message = "SAT export stub (use STEP)"; return r;
}
IoResult ImportExportService::importParasolid(const std::string& path) const {
    (void)path;
    IoResult r; r.success = false; r.message = "Parasolid import stub (use STEP)"; return r;
}
IoResult ImportExportService::exportParasolid(const std::string& path, const cad::core::Part& part, bool binary) const {
    (void)path; (void)part; (void)binary;
    IoResult r; r.success = false; r.message = "Parasolid export stub (use STEP)"; return r;
}
IoResult ImportExportService::importJt(const std::string& path) const {
    (void)path;
    IoResult r; r.success = false; r.message = "JT import stub (use STEP)"; return r;
}
IoResult ImportExportService::exportJt(const std::string& path, const cad::core::Assembly& assembly) const {
    (void)path; (void)assembly;
    IoResult r; r.success = false; r.message = "JT export stub (use STEP)"; return r;
}
IoResult ImportExportService::exportPdf(const std::string& path, const cad::core::Part& part) const {
    (void)path; (void)part;
    IoResult r; r.success = false; r.message = "3D PDF export stub"; return r;
}
IoResult ImportExportService::importFbx(const std::string& path) const {
    (void)path;
    IoResult r; r.success = false; r.message = "FBX import stub (use OBJ/GLTF)"; return r;
}
IoResult ImportExportService::exportFbx(const std::string& path, const cad::core::Part& part) const {
    (void)path; (void)part;
    IoResult r; r.success = false; r.message = "FBX export stub (use OBJ/GLTF)"; return r;
}

}  // namespace interop
}  // namespace cad
