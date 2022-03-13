add_program(
    TARGET ${TEST_EXECUTABLE_NAME}
    INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ../../clioption/test/utils/CliOptionTestUtils.h
    include/TestUtils/Event/EventTestUtils.h
)

add_subdirectory(src)