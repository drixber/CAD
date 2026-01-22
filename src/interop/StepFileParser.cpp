#include "StepFileParser.h"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

namespace cad {
namespace interop {

bool StepFileParser::parseFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return false;
    }
    
    std::string line;
    bool in_data_section = false;
    
    while (std::getline(file, line)) {
        if (line.find("ISO-10303-21") != std::string::npos) {
            continue;
        }
        if (line.find("HEADER;") != std::string::npos) {
            has_header_ = true;
            continue;
        }
        if (line.find("DATA;") != std::string::npos) {
            in_data_section = true;
            has_data_ = true;
            continue;
        }
        if (line.find("ENDSEC;") != std::string::npos) {
            if (in_data_section) {
                break;
            }
            continue;
        }
        
        if (in_data_section && line.find("=") != std::string::npos) {
            StepEntity entity = parseEntity(line);
            if (entity.id > 0) {
                entities_.push_back(entity);
            }
        }
    }
    
    file.close();
    return true;
}

StepEntity StepFileParser::parseEntity(const std::string& line) {
    StepEntity entity;
    
    size_t eq_pos = line.find("=");
    if (eq_pos == std::string::npos) {
        return entity;
    }
    
    std::string id_str = line.substr(0, eq_pos);
    id_str.erase(std::remove_if(id_str.begin(), id_str.end(), ::isspace), id_str.end());
    id_str.erase(std::remove(id_str.begin(), id_str.end(), '#'), id_str.end());
    
    try {
        entity.id = std::stoi(id_str);
    } catch (...) {
        return entity;
    }
    
    std::string rest = line.substr(eq_pos + 1);
    entity.type = extractEntityType(rest);
    entity.parameters = extractParameters(rest);
    
    return entity;
}

std::string StepFileParser::extractEntityType(const std::string& line) {
    size_t paren_pos = line.find("(");
    if (paren_pos == std::string::npos) {
        return "";
    }
    
    std::string type = line.substr(0, paren_pos);
    type.erase(std::remove_if(type.begin(), type.end(), ::isspace), type.end());
    return type;
}

std::map<std::string, std::string> StepFileParser::extractParameters(const std::string& line) {
    std::map<std::string, std::string> params;
    
    size_t open_paren = line.find("(");
    size_t close_paren = line.find_last_of(")");
    
    if (open_paren == std::string::npos || close_paren == std::string::npos) {
        return params;
    }
    
    std::string param_str = line.substr(open_paren + 1, close_paren - open_paren - 1);
    
    std::istringstream iss(param_str);
    std::string token;
    int param_index = 0;
    
    while (std::getline(iss, token, ',')) {
        token.erase(std::remove_if(token.begin(), token.end(), ::isspace), token.end());
        token.erase(std::remove(token.begin(), token.end(), '\''), token.end());
        params["param_" + std::to_string(param_index++)] = token;
    }
    
    return params;
}

}  // namespace interop
}  // namespace cad

