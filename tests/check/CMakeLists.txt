find_package(Check)

include_directories(
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/tests/check
    ${CHECK_INCLUDE_DIRS}
)

#
# Add directories with tests
#
add_subdirectory(action)
add_subdirectory(command)
add_subdirectory(compositor)
add_subdirectory(connection)
add_subdirectory(logger)
add_subdirectory(objects)
add_subdirectory(serialize)
add_subdirectory(session)
add_subdirectory(storage)
add_subdirectory(util)
add_subdirectory(values)
