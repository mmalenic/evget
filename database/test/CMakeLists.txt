set(TEST_EXECUTABLE_NAME databasetest)

add_executable(
        ${TEST_EXECUTABLE_NAME}
        sqlite/Connection.cpp
)

setup_testing(${TEST_EXECUTABLE_NAME} ${PROJECT_NAME})
