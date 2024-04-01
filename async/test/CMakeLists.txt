set(TEST_EXECUTABLE_NAME asynctest)

add_executable(
        ${TEST_EXECUTABLE_NAME}
        container/LockingVector.cpp
)

setup_testing(${TEST_EXECUTABLE_NAME} ${PROJECT_NAME})
