add_library(cad_modules
    drawings/DrawingService.cpp
    sheetmetal/SheetMetalService.cpp
    simulation/SimulationService.cpp
    patterns/PatternService.cpp
    direct/DirectEditService.cpp
    routing/RoutingService.cpp
    visualization/VisualizationService.cpp
    mbd/MbdService.cpp
    simplify/SimplifyService.cpp
)

target_include_directories(cad_modules
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)
