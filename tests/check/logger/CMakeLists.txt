add_executable(check_logger.test EXCLUDE_FROM_ALL
    logger_tests
)

target_link_libraries(check_logger.test
    ${CHECK_LIBRARY}
)

if (${CHECK_FOUND})
    add_test("build_check_logger.test" "${CMAKE_COMMAND}"
        --build ${CMAKE_BINARY_DIR}
        --target check_logger.test
    )
    add_test("check_logger.test" check_logger_test)
    set_tests_properties(check_logger.test PROPERTIES DEPENDS
        build_check_logger.test
    )
else()
    message("Check framework not found")
endif()
