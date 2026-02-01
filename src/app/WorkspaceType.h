#pragma once

namespace cad {
namespace app {

/** Arbeitsbereich / Workbench (§22.2 CATIA, §5.2 Umgebung): Wechsel Design/Blech/Zeichnung/Simulation/etc. */
enum class WorkspaceType {
    Design,       /** Part Design / 3D-Modell */
    Assembly,     /** Assembly Design / Zusammenbauen */
    Sketch,       /** Skizze */
    Drawing,      /** Drafting / Zeichnung */
    SheetMetal,   /** Blech */
    Weldment,     /** Schweiß */
    TubePipe,     /** Rohr & Schlauch */
    Frame,        /** Rahmen */
    Harness,      /** Kabel & Harness */
    Simulation,   /** Belastung / Dynamik */
    Surface,      /** GSD / Flächen */
    Freeform,     /** FreeStyle / Form */
    Cam,          /** Manufacture / CAM */
    Electronics,  /** Electronics */
    Generative,   /** Generative Design */
    Render,       /** Visualize / Render */
    Composites,   /** Composites */
    Dmu,          /** DMU / Digital Mock-Up */
    Kinematics    /** Kinematik */
};

}  // namespace app
}  // namespace cad
