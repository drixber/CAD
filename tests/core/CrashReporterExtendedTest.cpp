#include <gtest/gtest.h>
#include "core/crash/CrashReporter.h"

using namespace cad::core;

TEST(CrashReporterExtendedTest, StackTraceCapture) {
    CrashReporter& reporter = CrashReporter::instance();
    
    reporter.initialize("test_dumps");
    reporter.captureStackTrace();
    
    std::string trace = reporter.getLastStackTrace();
    ASSERT_FALSE(trace.empty());
}

TEST(CrashReporterExtendedTest, CrashDumpGeneration) {
    CrashReporter& reporter = CrashReporter::instance();
    
    reporter.initialize("test_dumps");
    reporter.captureStackTrace();
    
    bool success = reporter.generateCrashDump("test_dump.dmp");
    ASSERT_TRUE(success);
    
    std::remove("test_dump.dmp");
}

TEST(CrashReporterExtendedTest, ErrorReporting) {
    CrashReporter& reporter = CrashReporter::instance();
    
    reporter.setErrorReportingEnabled(true);
    ASSERT_TRUE(reporter.isErrorReportingEnabled());
    
    bool sent = reporter.sendErrorReport("https://example.com/report", "Test error");
    ASSERT_TRUE(sent || !reporter.isErrorReportingEnabled());
}

TEST(CrashReporterExtendedTest, SystemInfo) {
    CrashReporter& reporter = CrashReporter::instance();
    
    reporter.reportCrash("Test crash");
    
    std::string trace = reporter.getLastStackTrace();
    ASSERT_FALSE(trace.empty());
}
