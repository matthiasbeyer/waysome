# - Try to find the "check" framework
# Once done this will define
#  CHECK_FOUND - System has check
#  CHECK_INCLUDE_DIRS - The check include directories
#  CHECK_LIBRARIES - The libraries needed to use check
#  CHECK_DEFINITIONS - Compiler switches required for using check

find_package(PkgConfig)
pkg_check_modules(PC_CHECK QUIET check)

set(CHECK_DEFINITIONS ${PC_CHECK_CFLAGS_OTHER})

find_path(CHECK_INCLUDE_DIR check.h
    HINTS ${PC_CHECK_INCLUDEDIR} ${PC_CHECK_INCLUDE_DIRS})

find_library(CHECK_LIBRARY NAMES check
    HINTS ${PC_CHECK_LIBDIR} ${PC_CHECK_LIBRARY_DIRS})

set(CHECK_LIBRARIES ${CHECK_LIBRARY})
set(CHECK_INCLUDE_DIRS ${CHECK_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)

# handle the QUIETLY and REQUIRED arguments and set CHECK_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(Check DEFAULT_MSG CHECK_LIBRARY CHECK_INCLUDE_DIR)

mark_as_advanced(CHECK_INCLUDE_DIR CHECK_LIBRARY)


