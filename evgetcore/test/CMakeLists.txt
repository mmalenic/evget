add_program(
    TARGET ${EVGETCORE_TEST_EXECUTABLE_NAME} MAKE_EXECUTABLE
    INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/utils
    SRCS
    Event/MouseMoveTest.cpp
    Event/MouseClickTest.cpp
    Event/KeyTest.cpp
    Event/MouseScrollTest.cpp
    Event/DataTest.cpp
    Event/FieldTest.cpp
    utils/EventTestUtils.h
    utils/EventTestUtils.cpp
)

link_testing(EVGETCORE_TEST_EXECUTABLE_NAME EVGETCORE_LIBRARY_NAME)