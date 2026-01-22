#include "ImportExportService.h"

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
    ImportRequest request;
    request.path = path;
    request.format = FileFormat::Step;
    return importModel(request);
}

IoResult ImportExportService::importIges(const std::string& path) const {
    ImportRequest request;
    request.path = path;
    request.format = FileFormat::Iges;
    return importModel(request);
}

IoResult ImportExportService::importStl(const std::string& path) const {
    ImportRequest request;
    request.path = path;
    request.format = FileFormat::Stl;
    return importModel(request);
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
    ExportRequest request;
    request.path = path;
    request.format = FileFormat::Step;
    IoResult result = exportModel(request);
    if (result.success && ascii_mode) {
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
    ExportRequest request;
    request.path = path;
    request.format = FileFormat::Stl;
    IoResult result = exportModel(request);
    if (result.success && ascii_mode) {
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
    // In real implementation: would check file header/magic numbers
    FileFormat detected = detectFileFormat(path);
    return detected == expected_format;
}

FileFormat ImportExportService::detectFileFormat(const std::string& path) const {
    // In real implementation: would detect format from file extension or header
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
