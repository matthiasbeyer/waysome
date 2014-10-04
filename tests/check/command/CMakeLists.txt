add_executable(check_command.test EXCLUDE_FROM_ALL
    command_tests
)

target_link_libraries(check_command.test
    ${CHECK_LIBRARY}
)

if (${CHECK_FOUND})
    add_test("build_check_command.test" "${CMAKE_COMMAND}"
        --build ${CMAKE_BINARY_DIR}
        --target check_command.test
    )
    add_test("check_command.test" check_command_test)
    set_tests_properties(check_command.test PROPERTIES DEPENDS
        build_check_command.test
    )
else()
    message("Check framework not found")
endif()
