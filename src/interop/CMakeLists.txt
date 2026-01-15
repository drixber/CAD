add_library(cad_interop
    ImportExportService.cpp
)

target_include_directories(cad_interop
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)
