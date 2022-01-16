add_program(
    TARGET ${TEST_EXECUTABLE_NAME}
    INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    include/TestUtils/CommandLine/CommandLineTestUtils.h
    include/TestUtils/Event/Event.h
)

add_subdirectory(src)