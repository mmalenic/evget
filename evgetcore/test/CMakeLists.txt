add_program(
    TARGET ${EVGETCORE_TEST_EXECUTABLE_NAME} MAKE_EXECUTABLE
    INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/utils
    SRCS
    Event/Common/TimeTest.cpp
    Event/Button/CharacterTest.cpp
    Event/Pointer/ScrollUpTest.cpp
    Event/Pointer/ScrollDownTest.cpp
    Event/Pointer/ScrollLeftTest.cpp
    Event/Pointer/ScrollRightTest.cpp
    Event/Pointer/PositionXTest.cpp
    Event/Pointer/PositionYTest.cpp
    Event/Button/NameTest.cpp
    Event/Pointer/EllipseMajorInnerTest.cpp
    Event/Pointer/EllipseMajorOuterTest.cpp
    Event/Pointer/EllipseMinorInnerTest.cpp
    Event/Pointer/EllipseMinorOuterTest.cpp
    Event/Pointer/MultitouchTest.cpp
    Event/Pointer/OrientationTest.cpp
    Event/Button/ActionTest.cpp
    Event/Button/IdentifierTest.cpp
    Event/Pointer/PositionXOuterTest.cpp
    Event/Pointer/PositionYOuterTest.cpp
    Event/Common/TypeTest.cpp
    Event/TableDataTest.cpp
    Event/MouseMoveTest.cpp
    Event/MouseClickTest.cpp
    Event/KeyTest.cpp
    Event/MouseScrollTest.cpp
    Event/TouchTest.cpp
    Event/DataTest.cpp
    Event/FieldTest.cpp
    utils/EventTestUtils.h
    utils/EventTestUtils.cpp
)

program_dependencies(TARGET ${EVGETCORE_TEST_EXECUTABLE_NAME} DEP PRIVATE GTest COMPONENTS gtest gtest_main)
program_dependencies(TARGET ${EVGETCORE_TEST_EXECUTABLE_NAME} DEP PUBLIC ${EVGETCORE_LIBRARY_NAME} ONLY_LINK)

include(GoogleTest)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
gtest_discover_tests(${EVGETCORE_TEST_EXECUTABLE_NAME})