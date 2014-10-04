add_executable(check_values.test EXCLUDE_FROM_ALL
    values_tests
)

target_link_libraries(check_values.test
    ${CHECK_LIBRARY}
)

if (${CHECK_FOUND})
    add_test("build_check_values.test" "${CMAKE_COMMAND}"
        --build ${CMAKE_BINARY_DIR}
        --target check_values.test
    )
    add_test("check_values.test" check_values_test)
    set_tests_properties(check_values.test PROPERTIES DEPENDS
        build_check_values.test
    )
else()
    message("Check framework not found")
endif()
