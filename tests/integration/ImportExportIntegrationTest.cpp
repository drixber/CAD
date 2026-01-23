#include <gtest/gtest.h>
#include "interop/ImportExportService.h"
#include <fstream>
#include <cstdio>

using namespace cad::interop;

TEST(ImportExportIntegrationTest, StepImportExport) {
    ImportExportService service;
    
    std::string test_file = "test_assembly.step";
    std::ofstream file(test_file);
    file << "ISO-10303-21;\n";
    file << "HEADER;\n";
    file << "ENDSEC;\n";
    file << "DATA;\n";
    file << "ENDSEC;\n";
    file << "END-ISO-10303-21;\n";
    file.close();
    
    IoResult import_result = service.importStep(test_file);
    ASSERT_TRUE(import_result.success);
    
    IoResult export_result = service.exportStep("test_export.step", true);
    ASSERT_TRUE(export_result.success);
    
    std::remove(test_file.c_str());
    std::remove("test_export.step");
}

TEST(ImportExportIntegrationTest, ObjImportExport) {
    ImportExportService service;
    
    std::string test_file = "test_model.obj";
    std::ofstream file(test_file);
    file << "# OBJ test file\n";
    file << "v 0.0 0.0 0.0\n";
    file << "v 1.0 0.0 0.0\n";
    file << "v 1.0 1.0 0.0\n";
    file << "f 1 2 3\n";
    file.close();
    
    IoResult import_result = service.importObj(test_file);
    ASSERT_TRUE(import_result.success);
    
    IoResult export_result = service.exportObj("test_export.obj");
    ASSERT_TRUE(export_result.success);
    
    std::remove(test_file.c_str());
    std::remove("test_export.obj");
}

TEST(ImportExportIntegrationTest, GltfImportExport) {
    ImportExportService service;
    
    std::string test_file = "test_model.gltf";
    std::ofstream file(test_file);
    file << "{\n";
    file << "  \"asset\": {\n";
    file << "    \"version\": \"2.0\",\n";
    file << "    \"generator\": \"Test\"\n";
    file << "  }\n";
    file << "}\n";
    file.close();
    
    IoResult import_result = service.importGltf(test_file);
    ASSERT_TRUE(import_result.success);
    
    IoResult export_result = service.exportGltf("test_export.gltf", false);
    ASSERT_TRUE(export_result.success);
    
    std::remove(test_file.c_str());
    std::remove("test_export.gltf");
}
