#pragma once

#include <string>

namespace cad {
namespace core {

class CrashReporter {
public:
    static CrashReporter& instance();

    bool initialize(const std::string& dump_path);
    const std::string& dumpPath() const;

private:
    CrashReporter() = default;

    std::string dump_path_{};
};

}  // namespace core
}  // namespace cad
