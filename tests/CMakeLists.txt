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
endif()
