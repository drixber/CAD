if(CAD_BUILD_TESTS)
    add_executable(cad_smoke
        smoke/SmokeTest.cpp
    )

    target_link_libraries(cad_smoke
        PRIVATE
            cad_core
            cad_kernel
            cad_simulation
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
endif()
