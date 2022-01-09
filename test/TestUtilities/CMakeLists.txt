add_program(
    TARGET ${TEST_EXECUTABLE_NAME}
    INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/TestUtilities
    include/TestUtilities/CommandLine/CommandLineTestUtilities.h
)

add_subdirectory(src)