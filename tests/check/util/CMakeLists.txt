add_executable(check_util.test EXCLUDE_FROM_ALL
    util_tests
)

target_link_libraries(check_util.test
    ${CHECK_LIBRARY}
)

if (${CHECK_FOUND})
    add_test("build_check_util.test" "${CMAKE_COMMAND}"
        --build ${CMAKE_BINARY_DIR}
        --target check_util.test
    )
    add_test("check_util.test" check_util_test)
    set_tests_properties(check_util.test PROPERTIES DEPENDS
        build_check_util.test
    )
else()
    message("Check framework not found")
endif()
