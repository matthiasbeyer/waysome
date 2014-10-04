add_executable(check_connection.test EXCLUDE_FROM_ALL
    connection_tests
)

target_link_libraries(check_connection.test
    ${CHECK_LIBRARY}
)

if (${CHECK_FOUND})
    add_test("build_check_connection.test" "${CMAKE_COMMAND}"
        --build ${CMAKE_BINARY_DIR}
        --target check_connection.test
    )
    add_test("check_connection.test" check_connection_test)
    set_tests_properties(check_connection.test PROPERTIES DEPENDS
        build_check_connection.test
    )
else()
    message("Check framework not found")
endif()
