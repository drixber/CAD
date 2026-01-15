#include "CrashReporter.h"

#include <fstream>

#ifdef _WIN32
#include <windows.h>
#endif

namespace cad {
namespace core {

CrashReporter& CrashReporter::instance() {
    static CrashReporter reporter;
    return reporter;
}

bool CrashReporter::initialize(const std::string& dump_path) {
    dump_path_ = dump_path;
#ifdef _WIN32
    auto handler = [](EXCEPTION_POINTERS* info) -> LONG {
        const std::string& path = CrashReporter::instance().dumpPath();
        if (!path.empty()) {
            std::ofstream stream(path, std::ios::app);
            if (stream.is_open()) {
                stream << "Unhandled exception: code=0x" << std::hex
                       << info->ExceptionRecord->ExceptionCode << std::dec << "\n";
            }
        }
        return EXCEPTION_EXECUTE_HANDLER;
    };
    SetUnhandledExceptionFilter(handler);
    return true;
#else
    (void)dump_path;
    return false;
#endif
}

const std::string& CrashReporter::dumpPath() const {
    return dump_path_;
}

}  // namespace core
}  // namespace cad
