add_library(cad_app
    AuthConfig.cpp
    CADApplication.cpp
    AppController.cpp
    UpdateService.cpp
    HttpClient.cpp
    ProjectFileService.cpp
    UserAuthService.cpp
    LicenseService.cpp
    CommunityService.cpp
    PrinterService.cpp
    ai/AIService.cpp
    ai/OpenAIProvider.cpp
    ai/GrokProvider.cpp
    ai/AnthropicProvider.cpp
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

if(CAD_USE_EIGENER_KERN)
    target_link_libraries(cad_app PRIVATE cad_eigen_kernel)
    target_compile_definitions(cad_app PRIVATE CAD_USE_EIGENER_KERN=1)
endif()

if(CAD_USE_QT)
    find_package(Qt6 QUIET COMPONENTS Network Core)
    if(Qt6_FOUND)
        target_link_libraries(cad_app PRIVATE Qt6::Network Qt6::Core)
        target_compile_definitions(cad_app PRIVATE CAD_USE_QT_NETWORK=1)
    endif()
endif()

if(DEFINED APP_VERSION)
    target_compile_definitions(cad_app PRIVATE CAD_APP_VERSION="${APP_VERSION}")
else()
    target_compile_definitions(cad_app PRIVATE CAD_APP_VERSION="v0.0.0")
endif()

