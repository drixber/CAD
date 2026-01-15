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
    )
endif()

add_library(cad_ui ${CAD_UI_SOURCES})

target_include_directories(cad_ui
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

if(CAD_USE_QT)
    find_package(Qt6 REQUIRED COMPONENTS Widgets)
    target_link_libraries(cad_ui PUBLIC Qt6::Widgets)
    target_compile_definitions(cad_ui PUBLIC CAD_USE_QT=1)
endif()
