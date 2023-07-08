set(TEST_EXECUTABLE_NAME evgetcoretest)

add_executable(
    ${TEST_EXECUTABLE_NAME}
    Event/MouseMoveTest.cpp
    Event/MouseClickTest.cpp
    Event/KeyTest.cpp
    Event/MouseScrollTest.cpp
    Event/DataTest.cpp
    Event/FieldTest.cpp
    Event/RelationTest.cpp
    utils/EventTestUtils.h
    utils/EventTestUtils.cpp
)
target_include_directories(${TEST_EXECUTABLE_NAME} PUBLIC utils)

setup_testing(${TEST_EXECUTABLE_NAME} ${PROJECT_NAME})
