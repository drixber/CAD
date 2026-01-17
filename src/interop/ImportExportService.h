#pragma once

#include <string>
#include <vector>

namespace cad {
namespace interop {

enum class FileFormat {
    Step,
    Iges,
    Stl,
    Dwg,
    Dxf,
    Sat,
    Rfa
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
};

}  // namespace interop
}  // namespace cad
