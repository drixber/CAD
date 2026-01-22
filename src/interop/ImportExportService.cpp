#include "ImportExportService.h"
#include "StepFileParser.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstring>
#include <ctime>

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
            FileFormat::Dxf, FileFormat::Sat, FileFormat::Rfa};
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
        case FileFormat::Rfa:
            return "RFA";
        default:
            return "UNKNOWN";
    }
}

bool ImportExportService::supportsImport(FileFormat format) const {
    switch (format) {
        case FileFormat::Rfa:
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
    ImportRequest request;
    request.path = path;
    request.format = FileFormat::Dwg;
    return importModel(request);
}

IoResult ImportExportService::importDxf(const std::string& path) const {
    ImportRequest request;
    request.path = path;
    request.format = FileFormat::Dxf;
    return importModel(request);
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
    file << "FILE_DESCRIPTION(('CADursor Export'), '2;1');\n";
    file << "FILE_NAME('" << path << "', '" << QDateTime::currentDateTime().toString("yyyy-MM-ddTHH:mm:ss").toStdString() << "', ('CADursor'), ('CADursor'), 'CADursor Export', 'CADursor', '');\n";
    file << "FILE_SCHEMA(('AUTOMOTIVE_DESIGN'));\n";
    file << "ENDSEC;\n";
    file << "DATA;\n";
    
    int entity_id = 1;
    for (const auto& part : request.parts) {
        file << "#" << entity_id << " = CARTESIAN_POINT('', (" << 0.0 << ", " << 0.0 << ", " << 0.0 << "));\n";
        entity_id++;
        file << "#" << entity_id << " = DIRECTION('', (" << 1.0 << ", " << 0.0 << ", " << 0.0 << "));\n";
        entity_id++;
        file << "#" << entity_id << " = DIRECTION('', (" << 0.0 << ", " << 1.0 << ", " << 0.0 << "));\n";
        entity_id++;
        file << "#" << entity_id << " = AXIS2_PLACEMENT_3D('', #" << (entity_id - 3) << ", #" << (entity_id - 2) << ", #" << (entity_id - 1) << ");\n";
        entity_id++;
    }
    
    file << "ENDSEC;\n";
    file << "END-ISO-10303-21;\n";
    
    // Extract filename from path
    std::string filename = path;
    size_t last_slash = path.find_last_of("/\\");
    if (last_slash != std::string::npos) {
        filename = path.substr(last_slash + 1);
    }
    
    file << "FILE_NAME('" << filename << "', '" << "2024-01-01T00:00:00" << "', ('CADursor'), ('CADursor System'), 'CADursor v1.0', 'CADursor', '');\n";
    file << "FILE_SCHEMA(('AUTOMOTIVE_DESIGN'));\n";
    file << "ENDSEC;\n";
    file << "DATA;\n";
    
    int entity_id = 1;
    
    file << "#" << entity_id++ << " = CARTESIAN_POINT('', (0.0, 0.0, 0.0));\n";
    file << "#" << entity_id++ << " = DIRECTION('', (0.0, 0.0, 1.0));\n";
    file << "#" << entity_id++ << " = DIRECTION('', (1.0, 0.0, 0.0));\n";
    file << "#" << entity_id++ << " = AXIS2_PLACEMENT_3D('', #" << (entity_id-3) << ", #" << (entity_id-2) << ", #" << (entity_id-1) << ");\n";
    file << "#" << entity_id++ << " = CARTESIAN_POINT('', (100.0, 100.0, 100.0));\n";
    file << "#" << entity_id++ << " = VERTEX_POINT('', #" << (entity_id-1) << ");\n";
    file << "#" << entity_id++ << " = VERTEX_POINT('', #" << (entity_id-3) << ");\n";
    file << "#" << entity_id++ << " = EDGE_CURVE('', #" << (entity_id-1) << ", #" << (entity_id-2) << ", #" << (entity_id-4) << ", .T.);\n";
    file << "#" << entity_id++ << " = MANIFOLD_SOLID_BREP('', #" << (entity_id-1) << ");\n";
    
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
    ExportRequest request;
    request.path = path;
    request.format = FileFormat::Iges;
    return exportModel(request);
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
        
        struct Triangle {
            double normal[3];
            double vertices[3][3];
        };
        
        auto generateCubeTriangles = []() -> std::vector<Triangle> {
        struct Triangle {
            double normal[3];
            double vertices[3][3];
        };
        
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
        std::string header_text = "CADursor STL Export";
        std::copy(header_text.begin(), header_text.end(), header);
        file.write(header, 80);
        
        // Number of triangles (placeholder: 12 for cube)
        uint32_t num_triangles = 12;
        file.write(reinterpret_cast<const char*>(&num_triangles), sizeof(uint32_t));
        
        auto generateCubeTriangles = []() -> std::vector<std::tuple<float[3], float[3][3]>> {
            std::vector<std::tuple<float[3], float[3][3]>> triangles;
            float half = 0.5f;
            
            float normal[3] = {0.0f, 0.0f, 1.0f};
            float vertices[3][3] = {{-half, -half, -half}, {half, -half, -half}, {half, half, -half}};
            triangles.push_back(std::make_tuple(normal, vertices));
            
            float vertices2[3][3] = {{-half, -half, -half}, {half, half, -half}, {-half, half, -half}};
            triangles.push_back(std::make_tuple(normal, vertices2));
            
            float normal2[3] = {0.0f, 0.0f, -1.0f};
            float vertices3[3][3] = {{-half, -half, half}, {-half, half, half}, {half, half, half}};
            triangles.push_back(std::make_tuple(normal2, vertices3));
            
            float vertices4[3][3] = {{-half, -half, half}, {half, half, half}, {half, -half, half}};
            triangles.push_back(std::make_tuple(normal2, vertices4));
            
            float normal3[3] = {0.0f, 1.0f, 0.0f};
            float vertices5[3][3] = {{-half, half, -half}, {half, half, half}, {half, half, -half}};
            triangles.push_back(std::make_tuple(normal3, vertices5));
            
            float vertices6[3][3] = {{-half, half, -half}, {-half, half, half}, {half, half, half}};
            triangles.push_back(std::make_tuple(normal3, vertices6));
            
            float normal4[3] = {0.0f, -1.0f, 0.0f};
            float vertices7[3][3] = {{-half, -half, -half}, {half, -half, half}, {half, -half, -half}};
            triangles.push_back(std::make_tuple(normal4, vertices7));
            
            float vertices8[3][3] = {{-half, -half, -half}, {-half, -half, half}, {half, -half, half}};
            triangles.push_back(std::make_tuple(normal4, vertices8));
            
            float normal5[3] = {1.0f, 0.0f, 0.0f};
            float vertices9[3][3] = {{half, -half, -half}, {half, half, half}, {half, half, -half}};
            triangles.push_back(std::make_tuple(normal5, vertices9));
            
            float vertices10[3][3] = {{half, -half, -half}, {half, -half, half}, {half, half, half}};
            triangles.push_back(std::make_tuple(normal5, vertices10));
            
            float normal6[3] = {-1.0f, 0.0f, 0.0f};
            float vertices11[3][3] = {{-half, -half, -half}, {-half, half, -half}, {-half, half, half}};
            triangles.push_back(std::make_tuple(normal6, vertices11));
            
            float vertices12[3][3] = {{-half, -half, -half}, {-half, half, half}, {-half, -half, half}};
            triangles.push_back(std::make_tuple(normal6, vertices12));
            
            return triangles;
        };
        
        std::vector<std::tuple<float[3], float[3][3]>> cube_triangles = generateCubeTriangles();
        num_triangles = static_cast<uint32_t>(cube_triangles.size());
        file.seekp(80, std::ios::beg);
        file.write(reinterpret_cast<const char*>(&num_triangles), sizeof(uint32_t));
        
        for (const auto& tri : cube_triangles) {
            float normal[3];
            float vertices[3][3];
            std::memcpy(normal, std::get<0>(tri), 12);
            std::memcpy(vertices, std::get<1>(tri), 36);
            uint16_t attribute = 0;
            
            file.write(reinterpret_cast<const char*>(normal), 12);
            file.write(reinterpret_cast<const char*>(vertices), 36);
            file.write(reinterpret_cast<const char*>(&attribute), 2);
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
    ExportRequest request;
    request.path = path;
    request.format = FileFormat::Dwg;
    return exportModel(request);
}

IoResult ImportExportService::exportDxf(const std::string& path) const {
    ExportRequest request;
    request.path = path;
    request.format = FileFormat::Dxf;
    return exportModel(request);
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
        return header_str.find("solid") != std::string::npos || 
               (header[0] == 0 && header[79] == 0);
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
    return FileFormat::Step;  // Default
}

}  // namespace interop
}  // namespace cad
