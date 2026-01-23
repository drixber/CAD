#pragma once

#include <string>
#include <vector>
#include "../core/Modeler/Assembly.h"

namespace cad {
namespace interop {

enum class FileFormat {
    Step,
    Iges,
    Stl,
    Dwg,
    Dxf,
    Sat,
    Rfa,
    Obj,
    Ply,
    ThreeMf,
    Gltf,
    Glb
};

struct ImportRequest {
    std::string path;
    FileFormat format;
};

struct ExportRequest {
    std::string path;
    FileFormat format;
};

struct IoResult {
    bool success{false};
    std::string message;
};

class ImportExportService {
public:
    IoResult importModel(const ImportRequest& request) const;
    IoResult exportModel(const ExportRequest& request) const;
    std::vector<FileFormat> supportedFormats() const;
    IoResult exportBimRfa(const std::string& path) const;
    std::string formatLabel(FileFormat format) const;
    bool supportsImport(FileFormat format) const;
    bool supportsExport(FileFormat format) const;
    
    // Enhanced import/export
    IoResult importStep(const std::string& path) const;
    IoResult importIges(const std::string& path) const;
    IoResult importStl(const std::string& path) const;
    IoResult importDwg(const std::string& path) const;
    IoResult importDxf(const std::string& path) const;
    IoResult exportStep(const std::string& path, bool ascii_mode) const;
    IoResult exportIges(const std::string& path) const;
    IoResult exportStl(const std::string& path, bool ascii_mode) const;
    IoResult exportDwg(const std::string& path) const;
    IoResult exportDxf(const std::string& path) const;
    IoResult importObj(const std::string& path) const;
    IoResult exportObj(const std::string& path) const;
    IoResult importPly(const std::string& path) const;
    IoResult exportPly(const std::string& path) const;
    IoResult import3mf(const std::string& path) const;
    IoResult export3mf(const std::string& path) const;
    IoResult importGltf(const std::string& path) const;
    IoResult exportGltf(const std::string& path, bool binary = false) const;
    
    // Batch operations
    IoResult importMultiple(const std::vector<ImportRequest>& requests) const;
    IoResult exportMultiple(const std::vector<ExportRequest>& requests) const;
    
    // Format validation
    bool validateFileFormat(const std::string& path, FileFormat expected_format) const;
    FileFormat detectFileFormat(const std::string& path) const;
    
    // Assembly import/export
    cad::core::Assembly importStepToAssembly(const std::string& path) const;
    IoResult exportAssemblyToStep(const std::string& path, const cad::core::Assembly& assembly, bool ascii_mode = true) const;
    cad::core::Part importStlToPart(const std::string& path) const;
    IoResult exportPartToStl(const std::string& path, const cad::core::Part& part, bool ascii_mode = true) const;
};

}  // namespace interop
}  // namespace cad
