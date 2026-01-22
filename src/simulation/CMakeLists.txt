add_library(cad_simulation
    SimulationKernel.cpp
)

target_include_directories(cad_simulation
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(cad_simulation
    PUBLIC
        cad_core
)
