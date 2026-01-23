add_library(cad_app
    CADApplication.cpp
    AppController.cpp
    UpdateService.cpp
    UpdateChecker.cpp
    HttpClient.cpp
    ProjectFileService.cpp
    UserAuthService.cpp
    ai/AIService.cpp
    ai/OpenAIProvider.cpp
    ai/ContextManager.cpp
    ai/PromptBuilder.cpp
    UpdateInstaller.cpp
)

target_include_directories(cad_app
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(cad_app
    PUBLIC
        cad_core
        cad_ui
        cad_modules
        cad_interop
)

if(CAD_USE_QT)
    find_package(Qt6 QUIET COMPONENTS Network)
    if(Qt6_FOUND)
        target_link_libraries(cad_app PUBLIC Qt6::Network)
    else()
        find_package(Qt5 REQUIRED COMPONENTS Network)
        target_link_libraries(cad_app PUBLIC Qt5::Network)
    endif()
endif()
