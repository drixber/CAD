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
    Parasolid,  /** Parasolid (.x_t/.x_b) Stub */
    Jt,         /** JT Open Stub */
    Pdf,        /** 3D PDF / Zeichnungsexport Stub */
    Fbx,        /** FBX Stub */
    Rfa,
    Obj,
    Ply,
    ThreeMf,
    Gltf,
    Glb,
    /** SolidWorks (19.1): Stub – optional über STEP/neutral. */
    SldPrt,
    SldAsm,
    SldDrw,
    /** Creo (20.1): Stub – optional. */
    CreoPrt,
    CreoAsm,
    CreoDrw,
    /** CATIA (22.1): Stub – optional. */
    CatPart,
    CatProduct,
    CatDrawing
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

    /** SolidWorks (19.1): Stub – Import/Export .SLDPRT/.SLDASM/.SLDDRW (optional/STEP). */
    IoResult importSldPrt(const std::string& path) const;
    IoResult importSldAsm(const std::string& path) const;
    IoResult exportSldPrt(const std::string& path, const cad::core::Part& part) const;
    IoResult exportSldAsm(const std::string& path, const cad::core::Assembly& assembly) const;
    /** Creo (20.1): Stub – .PRT/.ASM/.DRW. */
    IoResult importCreoPrt(const std::string& path) const;
    IoResult importCreoAsm(const std::string& path) const;
    IoResult exportCreoPrt(const std::string& path, const cad::core::Part& part) const;
    IoResult exportCreoAsm(const std::string& path, const cad::core::Assembly& assembly) const;
    /** CATIA (22.1): Stub – .CATPart/.CATProduct/.CATDrawing. */
    IoResult importCatPart(const std::string& path) const;
    IoResult importCatProduct(const std::string& path) const;
    IoResult exportCatPart(const std::string& path, const cad::core::Part& part) const;
    IoResult exportCatProduct(const std::string& path, const cad::core::Assembly& assembly) const;

    /** SAT, Parasolid, JT, PDF, FBX Stubs. */
    IoResult importSat(const std::string& path) const;
    IoResult exportSat(const std::string& path, const cad::core::Part& part) const;
    IoResult importParasolid(const std::string& path) const;
    IoResult exportParasolid(const std::string& path, const cad::core::Part& part, bool binary = true) const;
    IoResult importJt(const std::string& path) const;
    IoResult exportJt(const std::string& path, const cad::core::Assembly& assembly) const;
    IoResult exportPdf(const std::string& path, const cad::core::Part& part) const;
    IoResult importFbx(const std::string& path) const;
    IoResult exportFbx(const std::string& path, const cad::core::Part& part) const;
};

}  // namespace interop
}  // namespace cad
