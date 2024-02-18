set(TEST_EXECUTABLE_NAME databasetest)

add_executable(
        ${TEST_EXECUTABLE_NAME}
        sqlite/Connection.cpp
        common/ConnectionTest.cpp
        common/ConnectionTest.h
)
target_include_directories(${TEST_EXECUTABLE_NAME} PUBLIC ccommon)

setup_testing(${TEST_EXECUTABLE_NAME} ${PROJECT_NAME})
