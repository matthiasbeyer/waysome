add_executable(check_action.test EXCLUDE_FROM_ALL
    action_tests
)

target_link_libraries(check_action.test
    ${CHECK_LIBRARY}
)

if (${CHECK_FOUND})
    add_test("build_check_action.test" "${CMAKE_COMMAND}"
        --build ${CMAKE_BINARY_DIR}
        --target check_action.test
    )
    add_test("check_action.test" check_action_test)
    set_tests_properties(check_action.test PROPERTIES DEPENDS
        build_check_action.test
    )
else()
    message("Check framework not found")
endif()
