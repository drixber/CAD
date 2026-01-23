#include <gtest/gtest.h>
#include "core/Modeler/Modeler.h"
#include "core/Modeler/Sketch.h"
#include "core/Modeler/Part.h"
#include "interop/ImportExportService.h"
#include <QApplication>

using namespace cad::core;
using namespace cad::interop;

class EndToEndTest : public ::testing::Test {
protected:
    void SetUp() override {
        int argc = 0;
        char** argv = nullptr;
        app_ = std::make_unique<QApplication>(argc, argv);
    }
    
    void TearDown() override {
        app_.reset();
    }
    
    std::unique_ptr<QApplication> app_;
};

TEST_F(EndToEndTest, CreatePartFromSketch) {
    Modeler modeler;
    
    Sketch sketch("TestSketch");
    sketch.addLine("line1", 0.0, 0.0, 10.0, 0.0);
    sketch.addLine("line2", 10.0, 0.0, 10.0, 10.0);
    sketch.addLine("line3", 10.0, 10.0, 0.0, 10.0);
    sketch.addLine("line4", 0.0, 10.0, 0.0, 0.0);
    
    bool valid = modeler.validateSketch(sketch);
    ASSERT_TRUE(valid);
    
    Part part = modeler.createPart(sketch);
    ASSERT_FALSE(part.name().empty());
}

TEST_F(EndToEndTest, ImportExportWorkflow) {
    ImportExportService service;
    
    std::string test_file = "test_workflow.step";
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
    
    IoResult export_result = service.exportStep("test_export_workflow.step", true);
    ASSERT_TRUE(export_result.success);
    
    std::remove(test_file.c_str());
    std::remove("test_export_workflow.step");
}
