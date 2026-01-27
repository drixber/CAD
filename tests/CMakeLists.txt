if(CAD_BUILD_TESTS)
    # Find or fetch GTest
    find_package(GTest QUIET)
    if(NOT GTest_FOUND)
        include(FetchContent)
        FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG v1.14.0
        )
        # For Windows: Prevent overriding the parent project's compiler/linker settings
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(googletest)
    endif()
    
    add_executable(cad_smoke
        smoke/SmokeTest.cpp
    )

    target_link_libraries(cad_smoke
        PRIVATE
            cad_core
            cad_kernel
            cad_simulation
            cad_interop
    )
    
    # Core Modeler Tests
    add_executable(modeler_test
        core/ModelerTest.cpp
    )
    
    target_link_libraries(modeler_test
        PRIVATE
            cad_core
    )
    
    target_include_directories(modeler_test
        PRIVATE
            ${CMAKE_SOURCE_DIR}/src
    )
    
    # Module Tests
    add_executable(pattern_service_test
        modules/PatternServiceTest.cpp
    )
    
    target_link_libraries(pattern_service_test
        PRIVATE
            cad_modules
            GTest::gtest
            GTest::gtest_main
    )
    
    target_include_directories(pattern_service_test
        PRIVATE
            ${CMAKE_SOURCE_DIR}/src
    )
    
    add_executable(simplify_service_test
        modules/SimplifyServiceTest.cpp
    )
    
    target_link_libraries(simplify_service_test
        PRIVATE
            cad_modules
            GTest::gtest
            GTest::gtest_main
    )
    
    target_include_directories(simplify_service_test
        PRIVATE
            ${CMAKE_SOURCE_DIR}/src
    )
    
    add_executable(visualization_service_test
        modules/VisualizationServiceTest.cpp
    )
    
    target_link_libraries(visualization_service_test
        PRIVATE
            cad_modules
            GTest::gtest
            GTest::gtest_main
    )
    
    target_include_directories(visualization_service_test
        PRIVATE
            ${CMAKE_SOURCE_DIR}/src
    )
    
    add_executable(mbd_service_test
        modules/MbdServiceTest.cpp
    )
    
    target_link_libraries(mbd_service_test
        PRIVATE
            cad_modules
            GTest::gtest
            GTest::gtest_main
    )
    
    target_include_directories(mbd_service_test
        PRIVATE
            ${CMAKE_SOURCE_DIR}/src
    )
    
    # Core Tests
    add_executable(crash_reporter_test
        core/CrashReporterTest.cpp
    )
    
    target_link_libraries(crash_reporter_test
        PRIVATE
            cad_core
            GTest::gtest
            GTest::gtest_main
    )
    
    target_include_directories(crash_reporter_test
        PRIVATE
            ${CMAKE_SOURCE_DIR}/src
    )
    
    add_executable(sheetmetal_service_test
        modules/SheetMetalServiceTest.cpp
    )
    
    target_link_libraries(sheetmetal_service_test
        PRIVATE
            cad_modules
            GTest::gtest
            GTest::gtest_main
    )
    
    target_include_directories(sheetmetal_service_test
        PRIVATE
            ${CMAKE_SOURCE_DIR}/src
    )
    
    add_executable(routing_service_test
        modules/RoutingServiceTest.cpp
    )
    
    target_link_libraries(routing_service_test
        PRIVATE
            cad_modules
            GTest::gtest
            GTest::gtest_main
    )
    
    target_include_directories(routing_service_test
        PRIVATE
            ${CMAKE_SOURCE_DIR}/src
    )
    
    add_executable(direct_edit_service_test
        modules/DirectEditServiceTest.cpp
    )
    
    target_link_libraries(direct_edit_service_test
        PRIVATE
            cad_modules
            GTest::gtest
            GTest::gtest_main
    )
    
    target_include_directories(direct_edit_service_test
        PRIVATE
            ${CMAKE_SOURCE_DIR}/src
    )
    
    add_executable(update_service_test
        app/UpdateServiceTest.cpp
    )
    
    target_link_libraries(update_service_test
        PRIVATE
            cad_app
            GTest::gtest
            GTest::gtest_main
    )
    
    target_include_directories(update_service_test
        PRIVATE
            ${CMAKE_SOURCE_DIR}/src
    )
    
    # Integration Tests
    add_executable(freecad_integration_test
        integration/FreeCADIntegrationTest.cpp
    )
    
    target_link_libraries(freecad_integration_test
        PRIVATE
            cad_core
            GTest::gtest
            GTest::gtest_main
    )
    
    target_include_directories(freecad_integration_test
        PRIVATE
            ${CMAKE_SOURCE_DIR}/src
    )
    
    add_executable(import_export_integration_test
        integration/ImportExportIntegrationTest.cpp
    )
    
    target_link_libraries(import_export_integration_test
        PRIVATE
            cad_interop
            GTest::gtest
            GTest::gtest_main
    )
    
    target_include_directories(import_export_integration_test
        PRIVATE
            ${CMAKE_SOURCE_DIR}/src
    )
    
    if(CAD_USE_QT)
        find_package(Qt6 QUIET COMPONENTS Core Widgets)
        if(Qt6_FOUND)
            add_executable(viewport_integration_test
                integration/ViewportIntegrationTest.cpp
            )
            
            target_link_libraries(viewport_integration_test
                PRIVATE
                    cad_ui
                    GTest::gtest
                    GTest::gtest_main
                    Qt6::Core
                    Qt6::Widgets
            )
            
            target_include_directories(viewport_integration_test
                PRIVATE
                    ${CMAKE_SOURCE_DIR}/src
            )
        endif()
    endif()
    
    add_executable(end_to_end_test
        integration/EndToEndTest.cpp
    )
    
    target_link_libraries(end_to_end_test
        PRIVATE
            cad_core
            cad_interop
            GTest::gtest
            GTest::gtest_main
    )
    
    if(CAD_USE_QT)
        find_package(Qt6 QUIET COMPONENTS Core Widgets)
        if(Qt6_FOUND)
            target_link_libraries(end_to_end_test
                PRIVATE
                    Qt6::Core
                    Qt6::Widgets
            )
        endif()
    endif()
    
    target_include_directories(end_to_end_test
        PRIVATE
            ${CMAKE_SOURCE_DIR}/src
    )
    
    # Extended Tests
    add_executable(pattern_service_extended_test
        modules/PatternServiceExtendedTest.cpp
    )
    
    target_link_libraries(pattern_service_extended_test
        PRIVATE
            cad_modules
            GTest::gtest
            GTest::gtest_main
    )
    
    target_include_directories(pattern_service_extended_test
        PRIVATE
            ${CMAKE_SOURCE_DIR}/src
    )
    
    add_executable(simplify_service_extended_test
        modules/SimplifyServiceExtendedTest.cpp
    )
    
    target_link_libraries(simplify_service_extended_test
        PRIVATE
            cad_modules
            GTest::gtest
            GTest::gtest_main
    )
    
    target_include_directories(simplify_service_extended_test
        PRIVATE
            ${CMAKE_SOURCE_DIR}/src
    )
    
    add_executable(visualization_service_extended_test
        modules/VisualizationServiceExtendedTest.cpp
    )
    
    target_link_libraries(visualization_service_extended_test
        PRIVATE
            cad_modules
            GTest::gtest
            GTest::gtest_main
    )
    
    target_include_directories(visualization_service_extended_test
        PRIVATE
            ${CMAKE_SOURCE_DIR}/src
    )
    
    add_executable(mbd_service_extended_test
        modules/MbdServiceExtendedTest.cpp
    )
    
    target_link_libraries(mbd_service_extended_test
        PRIVATE
            cad_modules
            GTest::gtest
            GTest::gtest_main
    )
    
    target_include_directories(mbd_service_extended_test
        PRIVATE
            ${CMAKE_SOURCE_DIR}/src
    )
    
    add_executable(undo_stack_extended_test
        core/UndoStackExtendedTest.cpp
    )
    
    target_link_libraries(undo_stack_extended_test
        PRIVATE
            cad_core
            GTest::gtest
            GTest::gtest_main
    )
    
    target_include_directories(undo_stack_extended_test
        PRIVATE
            ${CMAKE_SOURCE_DIR}/src
    )
    
    add_executable(crash_reporter_extended_test
        core/CrashReporterExtendedTest.cpp
    )
    
    target_link_libraries(crash_reporter_extended_test
        PRIVATE
            cad_core
            GTest::gtest
            GTest::gtest_main
    )
    
    target_include_directories(crash_reporter_extended_test
        PRIVATE
            ${CMAKE_SOURCE_DIR}/src
    )
endif()
