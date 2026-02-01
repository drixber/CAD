#include "PlasticsService.h"

namespace cad {
namespace modules {

PlasticsService::FillResult PlasticsService::runFillAnalysis(const std::string&) const {
    return FillResult{};
}
bool PlasticsService::runWeldLineAnalysis(const std::string&) const { return false; }
bool PlasticsService::runSinkMarkAnalysis(const std::string&) const { return false; }

}  // namespace modules
}  // namespace cad
