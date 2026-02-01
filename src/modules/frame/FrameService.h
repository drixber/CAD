#pragma once

#include <string>
#include <vector>
#include <map>

namespace cad {
namespace modules {

/** Profil-Typ (DIN/ISO/EN): Rechteck, Rohr, U, I, L. */
enum class FrameProfileType {
    Rectangle,  /** Rechteckprofil */
    Tube,       /** Rundrohr / Quadratrohr */
    U,          /** U-Profil */
    I,          /** I-Träger */
    L            /** L-Profil */
};

/** Ein Eintrag im Profil-Katalog (Bibliothek). */
struct FrameProfileSpec {
    std::string id;
    std::string name;
    FrameProfileType type{FrameProfileType::Rectangle};
    std::string standard{"DIN"};  /** DIN, ISO, EN */
    double width_mm{0.0};        /** Breite / Außendurchmesser */
    double height_mm{0.0};        /** Höhe */
    double wall_mm{0.0};          /** Wandstärke (Rohr/U/I/L) */
    double area_mm2{0.0};         /** Querschnittsfläche (optional) */
};

/** Ein Rahmen-Member: Profil + Länge, Platzierung. */
struct FrameMember {
    std::string member_id;
    std::string profile_id;
    double length_mm{0.0};
    /** Startpunkt (x,y,z) für Einfügen entlang Linie/Pfad */
    double start_x{0.0}, start_y{0.0}, start_z{0.0};
    double end_x{0.0}, end_y{0.0}, end_z{0.0};
    bool miter{false};   /** Automatische Gehrung an Ende */
    std::string group;   /** Strukturgruppe für Nummerierung */
};

/** Zuschnittliste: eine Länge + Menge (für Längenoptimierung). */
struct CutListEntry {
    std::string profile_id;
    std::string profile_name;
    double length_mm{0.0};
    int quantity{1};
    std::vector<std::string> member_ids;  /** Welche Member daraus geschnitten werden */
};

class FrameService {
public:
    /** Profil-Katalog: vordefinierte Profile (Rechteck, Rohr, U, I, L). */
    std::vector<FrameProfileSpec> getProfileCatalog() const;
    FrameProfileSpec getProfile(const std::string& profile_id) const;

    /** Rahmen in Baugruppe/Struktur: Member hinzufügen (entlang Linie/Pfad). */
    bool addMember(const std::string& assembly_or_frame_id, const FrameMember& member);
    bool addMemberAlongLine(const std::string& frame_id, const std::string& profile_id,
                            double x1, double y1, double z1, double x2, double y2, double z2,
                            bool miter = true, const std::string& group = "");
    std::vector<FrameMember> getMembers(const std::string& frame_id) const;

    /** Gehrung: automatische Kürzung/Verlängerung an Ecken. */
    void setMiterAngle(double degrees);
    double getMiterAngle() const;

    /** Zuschnittliste: nach Profil und Länge gruppiert. */
    std::vector<CutListEntry> getCutList(const std::string& frame_id) const;

    /** Längenoptimierung: Zuschnitt aus Standardlängen (z.B. 6000 mm), Verschnitt minimieren. */
    struct OptimizationResult {
        std::vector<CutListEntry> ordered_cuts;
        double total_waste_mm{0.0};
        int stock_length_mm{6000};
    };
    OptimizationResult optimizeLengths(const std::string& frame_id, int stock_length_mm = 6000) const;

private:
    mutable std::map<std::string, std::vector<FrameMember>> frame_members_;
    double miter_angle_deg_{45.0};
    std::vector<FrameProfileSpec> buildCatalog() const;
    double segmentLength(double x1, double y1, double z1, double x2, double y2, double z2) const;
};

}  // namespace modules
}  // namespace cad
