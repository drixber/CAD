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

}  // namespace interop
}  // namespace cad
