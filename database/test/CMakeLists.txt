set(TEST_EXECUTABLE_NAME databasetest)

add_executable(
        ${TEST_EXECUTABLE_NAME}
        sqlite/Connection.cpp
        sqlite/Query.cpp
        common/DatabaseTest.cpp
        common/DatabaseTest.h
        sqlite/Migrate.cpp
)
target_include_directories(${TEST_EXECUTABLE_NAME} PUBLIC common)

setup_testing(${TEST_EXECUTABLE_NAME} ${PROJECT_NAME})
