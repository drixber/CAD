add_library(cad_kernel
    KernelFacade.cpp
)

target_include_directories(cad_kernel
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(cad_kernel
    PUBLIC
        cad_core
)
