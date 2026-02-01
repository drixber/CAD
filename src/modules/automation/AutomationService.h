#pragma once

#include <string>
#include <vector>

namespace cad {
namespace modules {

/** Automatisierung / API (§17.10, §18.17): Makros, Skript-API (Python/JS), Add-Ins. Stub. */
class AutomationService {
public:
    bool runScript(const std::string& script_content, const std::string& language);
    bool recordMacro();
    bool stopMacro();
    std::string getRecordedMacro() const;
};

}  // namespace modules
}  // namespace cad
