#pragma once

#include <string>
#include <vector>
#include <map>

namespace cad {
namespace modules {

/** Nahttypen (Kehl-, Stumpf-, V-, U-, Punkt-, Kosmetisch). */
enum class WeldType {
    Fillet,    /** Kehlnaht */
    Butt,      /** Stumpfnaht */
    V,         /** V-Naht */
    U,         /** U-Naht */
    Spot,      /** Punktnaht */
    Cosmetic   /** Kosmetische Naht (nur Darstellung) */
};

/** Eine Schweißnaht: Typ, Größe, zugehörige Komponenten/Kanten. */
struct WeldJoint {
    std::string name;
    WeldType type{WeldType::Fillet};
    double length_mm{0.0};
    double size_mm{0.0};      /** Kehlnaht-Beinchen, Nahtdicke */
    double throat_mm{0.0};    /** Kehlnaht-Hals */
    std::string part_a;
    std::string part_b;
    std::string edge_a;
    std::string edge_b;
    /** Symbol für Zeichnung (ISO-Nummer, z.B. "ISO 2553") */
    std::string symbol;
};

/** Eintrag Schweißstückliste. */
struct WeldBomItem {
    std::string weld_type_name;
    std::string symbol;
    int quantity{1};
    double total_length_mm{0.0};
};

class WeldingService {
public:
    /** Naht zur Baugruppe hinzufügen. */
    bool addWeld(const std::string& assembly_id, const WeldJoint& weld);
    /** Alle Nähte einer Baugruppe. */
    std::vector<WeldJoint> getWelds(const std::string& assembly_id) const;
    /** Schweißstückliste (nach Nahttyp gruppiert). */
    std::vector<WeldBomItem> getWeldBom(const std::string& assembly_id) const;
    /** Nahtvolumen schätzen (Kehlnaht: 0.5 * size² * length). */
    double estimateWeldVolume(const WeldJoint& weld) const;
    /** Symbol-Text für Zeichnung (z.B. "Fillet 5"). */
    std::string getSymbolText(const WeldJoint& weld) const;

private:
    mutable std::map<std::string, std::vector<WeldJoint>> assembly_welds_;
};

}  // namespace modules
}  // namespace cad
