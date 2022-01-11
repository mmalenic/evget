set_local_parent(TESTUTILITIES_LIBRARY_NAME libtestutils)

add_program(
    TARGET ${TESTUTILITIES_LIBRARY_NAME} MAKE_LIBRARY
    INCLUDE_DIRS
    include
    SRCS
    include/TestUtilities/CommandLineTestUtilities.h
)

add_subdirectory(src)