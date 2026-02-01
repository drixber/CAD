#pragma once

#include <string>
#include <vector>

namespace cad {
namespace app {

class HttpClient;

struct PrinterProfile {
    std::string id;
    std::string name;
    std::string type;  // "octoprint" | "moonraker"
    std::string base_url;
    std::string api_key;
};

struct PrinterUploadResult {
    bool success{false};
    std::string message;
    int status_code{0};
};

class PrinterService {
public:
    PrinterService() = default;
    void setHttpClient(HttpClient* client) { http_client_ = client; }

    std::vector<PrinterProfile> getPrinters() const;
    bool addPrinter(const PrinterProfile& profile);
    bool removePrinter(const std::string& id);
    bool updatePrinter(const std::string& id, const PrinterProfile& profile);
    PrinterProfile getPrinter(const std::string& id) const;

    /** Test connection (GET version/connection endpoint). Returns success and optional message. */
    PrinterUploadResult testConnection(const std::string& printer_id) const;

    /** Upload STL file to printer. OctoPrint: POST /api/files/local; Moonraker: POST /server/files/upload. */
    PrinterUploadResult sendStlToPrinter(const std::string& printer_id, const std::string& stl_path,
                                          bool start_print = false) const;

private:
    HttpClient* http_client_{nullptr};
    void loadPrinters() const;
    void savePrinters() const;
    mutable std::vector<PrinterProfile> printers_;
    static std::string generateId();
};

}  // namespace app
}  // namespace cad
