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
endif()
