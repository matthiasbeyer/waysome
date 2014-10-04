add_executable(check_compositor.test EXCLUDE_FROM_ALL
    compositor_tests
)

target_link_libraries(check_compositor.test
    ${CHECK_LIBRARY}
)

if (${CHECK_FOUND})
    add_test("build_check_compositor.test" "${CMAKE_COMMAND}"
        --build ${CMAKE_BINARY_DIR}
        --target check_compositor.test
    )
    add_test("check_compositor.test" check_compositor_test)
    set_tests_properties(check_compositor.test PROPERTIES DEPENDS
        build_check_compositor.test
    )
else()
    message("Check framework not found")
endif()
