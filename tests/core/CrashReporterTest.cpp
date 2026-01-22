#include <gtest/gtest.h>
#include "core/crash/CrashReporter.h"
#include <fstream>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/stat.h>
#endif

using namespace cad::core;

TEST(CrashReporterTest, Initialize) {
    CrashReporter& reporter = CrashReporter::instance();
    
    bool initialized = reporter.initialize("test_crash.log");
    EXPECT_TRUE(initialized);
    
    std::string dump_path = reporter.dumpPath();
    EXPECT_EQ(dump_path, "test_crash.log");
}

TEST(CrashReporterTest, ReportCrash) {
    CrashReporter& reporter = CrashReporter::instance();
    reporter.initialize("test_crash_report.log");
    
    reporter.reportCrash("Test error message");
    
    std::string stack_trace = reporter.getLastStackTrace();
    EXPECT_FALSE(stack_trace.empty());
}

TEST(CrashReporterTest, CaptureStackTrace) {
    CrashReporter& reporter = CrashReporter::instance();
    reporter.initialize("test_stack.log");
    
    reporter.captureStackTrace();
    
    std::string stack_trace = reporter.getLastStackTrace();
    EXPECT_FALSE(stack_trace.empty());
}

TEST(CrashReporterTest, GenerateCrashDump) {
    CrashReporter& reporter = CrashReporter::instance();
    reporter.initialize("test_dump.log");
    reporter.captureStackTrace();
    
    bool generated = reporter.generateCrashDump("test_crash.dmp");
    EXPECT_TRUE(generated);
    
    std::ifstream test_file("test_crash.dmp");
    if (test_file.good()) {
        test_file.close();
        std::remove("test_crash.dmp");
    }
}

TEST(CrashReporterTest, Singleton) {
    CrashReporter& reporter1 = CrashReporter::instance();
    CrashReporter& reporter2 = CrashReporter::instance();
    
    EXPECT_EQ(&reporter1, &reporter2);
}

