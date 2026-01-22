add_library(cad_interop
    ImportExportService.cpp
    IoPipeline.cpp
    StepFileParser.cpp
)

target_include_directories(cad_interop
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src
)
