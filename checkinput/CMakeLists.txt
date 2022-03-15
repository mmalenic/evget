set_variable(CHECKINPUT_LIBRARY_NAME libcheckinput)
set_variable(CHECKINPUT_TEST_EXECUTABLE_NAME checkinputtest)

add_program(
    TARGET ${CHECKINPUT_LIBRARY_NAME} MAKE_LIBRARY
    FEATURE_REQUIRES fcntl.h linux/input.h
    FEATURE_CHECK CHECK_INPUT_HAS_DEPENDENCIES
    INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    SRCS
    include/checkinput/EventDevice.h
    include/checkinput/EventDeviceLister.h
)

add_subdirectory(src)

program_dependencies(TARGET ${CHECKINPUT_LIBRARY_NAME} DEP PUBLIC fmt 8.0.1)
program_dependencies(TARGET ${CHECKINPUT_LIBRARY_NAME} DEP PUBLIC spdlog 1.9.2)
program_dependencies(TARGET ${CHECKINPUT_LIBRARY_NAME} DEP PUBLIC Boost 1.78 FIND_ALL COMPONENTS headers)

if (BUILD_TESTING)
    add_program(TARGET ${CHECKINPUT_TEST_EXECUTABLE_NAME} MAKE_EXECUTABLE)

    add_subdirectory(test)
    program_dependencies(TARGET ${CHECKINPUT_TEST_EXECUTABLE_NAME} DEP PRIVATE GTest COMPONENTS gtest gtest_main)
    program_dependencies(TARGET ${CHECKINPUT_TEST_EXECUTABLE_NAME} DEP PUBLIC ${CHECKINPUT_LIBRARY_NAME} ONLY_LINK)
endif()