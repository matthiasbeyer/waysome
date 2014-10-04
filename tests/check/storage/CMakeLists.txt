add_executable(check_storage.test EXCLUDE_FROM_ALL
    storage_tests
)

target_link_libraries(check_storage.test
    ${CHECK_LIBRARY}
)

if (${CHECK_FOUND})
    add_test("build_check_storage.test" "${CMAKE_COMMAND}"
        --build ${CMAKE_BINARY_DIR}
        --target check_storage.test
    )
    add_test("check_storage.test" check_storage_test)
    set_tests_properties(check_storage.test PROPERTIES DEPENDS
        build_check_storage.test
    )
else()
    message("Check framework not found")
endif()
