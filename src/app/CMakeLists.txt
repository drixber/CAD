add_library(cad_app
    CADApplication.cpp
    AppController.cpp
)

target_include_directories(cad_app
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(cad_app
    PUBLIC
        cad_core
        cad_ui
        cad_modules
        cad_interop
)
