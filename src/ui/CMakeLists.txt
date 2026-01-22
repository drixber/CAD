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
        styles/Styles.qrc
    )
endif()

add_library(cad_ui ${CAD_UI_SOURCES})

target_include_directories(cad_ui
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src
)

if(CAD_USE_QT)
    # Versuche zuerst Qt6, dann Qt5
    find_package(Qt6 QUIET COMPONENTS Widgets Core)
    if(Qt6_FOUND)
        set(CMAKE_AUTOMOC ON)
        target_link_libraries(cad_ui PUBLIC Qt6::Widgets Qt6::Core)
        target_compile_definitions(cad_ui PUBLIC CAD_USE_QT=1)
    else()
        find_package(Qt5 REQUIRED COMPONENTS Widgets Core)
        set(CMAKE_AUTOMOC ON)
        target_link_libraries(cad_ui PUBLIC Qt5::Widgets Qt5::Core)
        target_compile_definitions(cad_ui PUBLIC CAD_USE_QT=1)
    endif()
endif()
