#include "CrashReporter.h"

#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>

#ifdef _WIN32
#include <windows.h>
#include <dbghelp.h>
#pragma comment(lib, "dbghelp.lib")
#else
#include <execinfo.h>
#include <cxxabi.h>
#include <cstdlib>
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

void CrashReporter::reportCrash(const std::string& error_message) {
    std::string crash_file = dump_path_.empty() ? "crash_report.txt" : dump_path_;
    writeCrashInfo(crash_file, error_message);
    captureStackTrace();
}

void CrashReporter::captureStackTrace() {
    std::ostringstream trace;
    
#ifdef _WIN32
    void* stack[64];
    unsigned short frames = CaptureStackBackTrace(0, 64, stack, nullptr);
    
    HANDLE process = GetCurrentProcess();
    SymInitialize(process, nullptr, TRUE);
    
    SYMBOL_INFO* symbol = (SYMBOL_INFO*)calloc(sizeof(SYMBOL_INFO) + 256 * sizeof(char), 1);
    symbol->MaxNameLen = 255;
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
    
    for (unsigned short i = 0; i < frames; i++) {
        SymFromAddr(process, (DWORD64)(stack[i]), 0, symbol);
        trace << i << ": " << symbol->Name << " (0x" << std::hex << symbol->Address << std::dec << ")\n";
    }
    
    free(symbol);
    SymCleanup(process);
#else
    void* array[64];
    int size = backtrace(array, 64);
    char** messages = backtrace_symbols(array, size);
    
    for (int i = 0; i < size && messages != nullptr; ++i) {
        trace << i << ": " << messages[i] << "\n";
    }
    
    free(messages);
#endif
    
    last_stack_trace_ = trace.str();
}

std::string CrashReporter::getLastStackTrace() const {
    return last_stack_trace_;
}

bool CrashReporter::generateCrashDump(const std::string& dump_file_path) {
#ifdef _WIN32
    HANDLE hFile = CreateFileA(dump_file_path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE) {
        return false;
    }
    
    MINIDUMP_EXCEPTION_INFORMATION exception_info;
    exception_info.ThreadId = GetCurrentThreadId();
    exception_info.ExceptionPointers = nullptr;
    exception_info.ClientPointers = FALSE;
    
    bool success = MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile,
                                     MiniDumpNormal, &exception_info, nullptr, nullptr);
    
    CloseHandle(hFile);
    return success;
#else
    std::ofstream dump_file(dump_file_path);
    if (!dump_file.is_open()) {
        return false;
    }
    
    dump_file << "Crash Dump\n";
    dump_file << "Timestamp: " << getSystemInfo() << "\n";
    dump_file << "Stack Trace:\n" << last_stack_trace_ << "\n";
    
    dump_file.close();
    return true;
#endif
}

void CrashReporter::writeCrashInfo(const std::string& file_path, const std::string& error_message) {
    std::ofstream stream(file_path, std::ios::app);
    if (!stream.is_open()) {
        return;
    }
    
    std::time_t now = std::time(nullptr);
    std::tm* time_info = std::localtime(&now);
    
    stream << "=== Crash Report ===\n";
    stream << "Time: " << std::put_time(time_info, "%Y-%m-%d %H:%M:%S") << "\n";
    stream << "Error: " << error_message << "\n";
    stream << "System Info:\n" << getSystemInfo() << "\n";
    stream << "Stack Trace:\n" << last_stack_trace_ << "\n";
    stream << "===================\n\n";
    
    stream.close();
}

std::string CrashReporter::getSystemInfo() const {
    std::ostringstream info;
    
#ifdef _WIN32
    OSVERSIONINFO osvi;
    ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&osvi);
    
    info << "Windows " << osvi.dwMajorVersion << "." << osvi.dwMinorVersion << "\n";
    info << "Build: " << osvi.dwBuildNumber << "\n";
    
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    info << "CPU Cores: " << sysInfo.dwNumberOfProcessors << "\n";
    info << "Memory: " << (sysInfo.dwPageSize * sysInfo.dwNumberOfProcessors) / (1024 * 1024) << " MB\n";
#else
    info << "Unix-like system\n";
#endif
    
    return info.str();
}

bool CrashReporter::sendErrorReport(const std::string& server_url, const std::string& error_message) {
    if (!error_reporting_enabled_ || server_url.empty()) {
        return false;
    }
    
    // In real implementation, this would use HttpClient to POST error report
    // For now, we just log it
    std::string report_file = dump_path_.empty() ? "error_report.txt" : dump_path_ + "/error_report.txt";
    writeCrashInfo(report_file, error_message);
    
    error_reporting_url_ = server_url;
    return true;
}

void CrashReporter::setErrorReportingEnabled(bool enabled) {
    error_reporting_enabled_ = enabled;
}

bool CrashReporter::isErrorReportingEnabled() const {
    return error_reporting_enabled_;
}

std::string CrashReporter::demangleSymbol(const std::string& symbol) const {
#ifdef _WIN32
    return symbol;  // Windows symbols are usually not mangled
#else
    // In real implementation, use cxxabi::__cxa_demangle
    return symbol;
#endif
}

}  // namespace core
}  // namespace cad
