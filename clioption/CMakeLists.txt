set_and_get_variable(CLIOPTION_LIBRARY_NAME libclioption)
set_and_get_variable(CLIOPTION_TEST_EXECUTABLE_NAME clioptiontest)

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
    include/clioption/Parser.h
)

add_subdirectory(src)

program_dependencies(TARGET ${CLIOPTION_LIBRARY_NAME} DEP PUBLIC fmt ${FMT_VERSION})
program_dependencies(TARGET ${CLIOPTION_LIBRARY_NAME} DEP PUBLIC spdlog ${SPDLOG_VERSION})
program_dependencies(TARGET ${CLIOPTION_LIBRARY_NAME} DEP PUBLIC Boost ${BOOST_VERSION} FIND_ALL COMPONENTS program_options)

if (BUILD_TESTING)
    add_subdirectory(test)
endif ()