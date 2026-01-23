set(CAD_UI_SOURCES
    MainWindow.cpp
    RibbonLayout.cpp
    BrowserTree.cpp
    PropertyPanel.cpp
    CommandLine.cpp
)

if(CAD_USE_QT)
    list(APPEND CAD_UI_SOURCES
        qt/QtMainWindow.cpp
        qt/QtRibbon.cpp
        qt/QtBrowserTree.cpp
        qt/QtPropertyPanel.cpp
        qt/QtCommandLine.cpp
        qt/QtAgentConsole.cpp
        qt/QtAgentThoughts.cpp
        qt/QtViewport.cpp
        qt/QtLogPanel.cpp
        qt/QtPerformancePanel.cpp
        viewport/Viewport3D.cpp
        viewport/RenderEngine3D.cpp
        viewport/Coin3DIntegration.cpp
        styles/Styles.qrc
    )
endif()

add_library(cad_ui ${CAD_UI_SOURCES})

if(CAD_USE_QT)
    set_target_properties(cad_ui PROPERTIES
        AUTOMOC ON
        AUTORCC ON
        AUTOUIC ON
    )
endif()

target_include_directories(cad_ui
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src
)

if(CAD_USE_QT)
    find_package(Qt6 QUIET COMPONENTS Widgets Core OpenGL)
    if(Qt6_FOUND)
        set(CMAKE_AUTOMOC ON)
        set(CMAKE_AUTORCC ON)
        set(CMAKE_AUTOUIC ON)
        target_link_libraries(cad_ui PUBLIC Qt6::Widgets Qt6::Core Qt6::OpenGL)
        target_compile_definitions(cad_ui PUBLIC CAD_USE_QT=1)
    else()
        find_package(Qt5 REQUIRED COMPONENTS Widgets Core OpenGL)
        set(CMAKE_AUTOMOC ON)
        set(CMAKE_AUTORCC ON)
        set(CMAKE_AUTOUIC ON)
        target_link_libraries(cad_ui PUBLIC Qt5::Widgets Qt5::Core Qt5::OpenGL)
        target_compile_definitions(cad_ui PUBLIC CAD_USE_QT=1)
    endif()
endif()

option(CAD_USE_COIN3D "Enable Coin3D integration for 3D rendering" OFF)
if(CAD_USE_COIN3D)
    find_package(Coin QUIET)
    if(Coin_FOUND)
        target_link_libraries(cad_ui PUBLIC Coin::Coin)
        target_compile_definitions(cad_ui PUBLIC CAD_USE_COIN3D=1)
    else()
        message(WARNING "Coin3D not found. Install via vcpkg: vcpkg install coin3d")
    endif()
endif()

option(CAD_USE_OCCT "Enable OpenCASCADE Technology integration" OFF)
if(CAD_USE_OCCT)
    find_package(OpenCASCADE QUIET)
    if(OpenCASCADE_FOUND)
        target_link_libraries(cad_ui PUBLIC ${OpenCASCADE_LIBRARIES})
        target_include_directories(cad_ui PUBLIC ${OpenCASCADE_INCLUDE_DIR})
        target_compile_definitions(cad_ui PUBLIC CAD_USE_OCCT=1)
    else()
        message(WARNING "OpenCASCADE not found. Install via vcpkg: vcpkg install opencascade")
    endif()
endif()
