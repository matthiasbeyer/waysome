add_executable(check_serialize.test EXCLUDE_FROM_ALL
    serialize_tests
)

target_link_libraries(check_serialize.test
    ${CHECK_LIBRARY}
)

if (${CHECK_FOUND})
    add_test("build_check_serialize.test" "${CMAKE_COMMAND}"
        --build ${CMAKE_BINARY_DIR}
        --target check_serialize.test
    )
    add_test("check_serialize.test" check_serialize_test)
    set_tests_properties(check_serialize.test PROPERTIES DEPENDS
        build_check_serialize.test
    )
else()
    message("Check framework not found")
endif()
