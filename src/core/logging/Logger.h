#pragma once

#include <mutex>
#include <string>

namespace cad {
namespace core {

class Logger {
public:
    enum class Level {
        Debug,
        Info,
        Warn,
        Error
    };

    static Logger& instance();

    void initialize(const std::string& file_path);
    void setMinLevel(Level level);
    void log(Level level, const std::string& message);

private:
    Logger() = default;

    std::string levelToString(Level level) const;
    std::string timestamp() const;

    Level min_level_{Level::Info};
    std::string file_path_{};
    std::mutex mutex_{};
};

}  // namespace core
}  // namespace cad
