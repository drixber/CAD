find_package(pybind11 CONFIG REQUIRED)

pybind11_add_module(cadursor
    cad_pybind.cpp
)

target_link_libraries(cadursor
    PRIVATE
        cad_core
)
