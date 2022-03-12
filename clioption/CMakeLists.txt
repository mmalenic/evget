set_variable(CLIOPTION_LIBRARY_NAME libclioption)
set_variable(CLIOPTION_TEST_EXECUTABLE_NAME clioptiontest)

add_program(
    TARGET ${CLIOPTION_LIBRARY_NAME} MAKE_LIBRARY
    INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    SRCS
    include/clioption/AbstractOption.h
    include/clioption/InvalidCommandLineOption.h
    include/clioption/Option.h
    include/clioption/OptionBuilder.h
    include/clioption/OptionFlag.h
    include/clioption/OptionValidated.h
)

add_subdirectory(src)

program_dependencies(TARGET ${CLIOPTION_LIBRARY_NAME} DEP PUBLIC fmt 8.0.1)
program_dependencies(TARGET ${CLIOPTION_LIBRARY_NAME} DEP PUBLIC spdlog 1.9.2)
program_dependencies(TARGET ${CLIOPTION_LIBRARY_NAME} DEP PUBLIC Boost 1.78 FIND_ALL COMPONENTS program_options)

if (BUILD_TESTING)
    add_program(TARGET ${CLIOPTION_TEST_EXECUTABLE_NAME} MAKE_EXECUTABLE)

    add_subdirectory(test)
    program_dependencies(TARGET ${CLIOPTION_TEST_EXECUTABLE_NAME} DEP PRIVATE GTest COMPONENTS gtest gtest_main)
    program_dependencies(TARGET ${CLIOPTION_TEST_EXECUTABLE_NAME} DEP PUBLIC ${CLIOPTION_LIBRARY_NAME} ONLY_LINK)
endif()