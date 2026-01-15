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
