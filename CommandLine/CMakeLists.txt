set(COMMAND_LINE_LIBRARY_NAME libcommandline)
set(COMMAND_LINE_TEST_EXECUTABLE_NAME commandlinetest)

add_program_linux(
    TARGET ${COMMAND_LINE_LIBRARY_NAME}
    INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    SRCS
    include/CommandLine/AbstractOption.h
    include/CommandLine/InvalidCommandLineOption.h
    include/CommandLine/Option.h
    include/CommandLine/OptionBuilder.h
    include/CommandLine/OptionFlag.h
    include/CommandLine/OptionValidated.h
    include/CommandLine/Parser.h
)

add_subdirectory(src)

program_dependencies(TARGET ${COMMAND_LINE_LIBRARY_NAME} DEP PUBLIC fmt 8.0.1)
program_dependencies(TARGET ${COMMAND_LINE_LIBRARY_NAME} DEP PUBLIC spdlog 1.9.2)
program_dependencies(TARGET ${COMMAND_LINE_LIBRARY_NAME} DEP PUBLIC Boost 1.78 FIND_ALL COMPONENTS program_options)

if (BUILD_TESTING)
    add_program(TARGET ${COMMAND_LINE_TEST_EXECUTABLE_NAME} MAKE_EXECUTABLE)

    add_subdirectory(test)
    program_dependencies(TARGET ${COMMAND_LINE_TEST_EXECUTABLE_NAME} DEP PRIVATE GTest COMPONENTS gtest gtest_main)
endif()