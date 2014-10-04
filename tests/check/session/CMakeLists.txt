add_executable(check_session.test EXCLUDE_FROM_ALL
    session_tests
)

target_link_libraries(check_session.test
    ${CHECK_LIBRARY}
)

if (${CHECK_FOUND})
    add_test("build_check_session.test" "${CMAKE_COMMAND}"
        --build ${CMAKE_BINARY_DIR}
        --target check_session.test
    )
    add_test("check_session.test" check_session_test)
    set_tests_properties(check_session.test PROPERTIES DEPENDS
        build_check_session.test
    )
else()
    message("Check framework not found")
endif()
