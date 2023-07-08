set(TEST_EXECUTABLE_NAME evwatchsigtest)

add_executable(${TEST_EXECUTABLE_NAME} EventWatcherSignalTest.cpp)

setup_testing(${TEST_EXECUTABLE_NAME} ${PROJECT_NAME})
