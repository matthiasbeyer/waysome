add_executable(check_objects.test EXCLUDE_FROM_ALL
    objects_tests
)

target_link_libraries(check_objects.test
    ${CHECK_LIBRARY}
)

if (${CHECK_FOUND})
    add_test("build_check_objects.test" "${CMAKE_COMMAND}"
        --build ${CMAKE_BINARY_DIR}
        --target check_objects.test
    )
    add_test("check_objects.test" check_objects_test)
    set_tests_properties(check_objects.test PROPERTIES DEPENDS
        build_check_objects.test
    )
else()
    message("Check framework not found")
endif()
