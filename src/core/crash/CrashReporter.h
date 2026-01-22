#pragma once

#include <string>

namespace cad {
namespace core {

class CrashReporter {
public:
    static CrashReporter& instance();

    bool initialize(const std::string& dump_path);
    const std::string& dumpPath() const;
    
    void reportCrash(const std::string& error_message);
    void captureStackTrace();
    std::string getLastStackTrace() const;
    bool generateCrashDump(const std::string& dump_file_path);

private:
    CrashReporter() = default;

    std::string dump_path_{};
    std::string last_stack_trace_{};
    
    void writeCrashInfo(const std::string& file_path, const std::string& error_message);
    std::string getSystemInfo() const;
};

}  // namespace core
}  // namespace cad
