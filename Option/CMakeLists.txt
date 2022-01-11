set_local_parent(OPTION_LIBRARY_NAME liboption)
set_local_parent(OPTION_TEST_NAME optiontest)

add_program(
    TARGET ${OPTION_LIBRARY_NAME} MAKE_LIBRARY
    INCLUDE_DIRS
    include
    SRCS
    include/Option/InvalidCommandLineOption.h
    include/Option/Option.h
    include/Option/OptionBase.h
    include/Option/OptionBuilder.h
    include/Option/OptionFlag.h
    include/Option/OptionValidated.h
    include/Option/UnsupportedOperationException.h
)

message("${OPTION_LIBRARY_NAME} HERE")

program_dependencies(TARGET ${OPTION_LIBRARY_NAME} DEP PUBLIC Boost 1.77 FIND_ALL COMPONENTS program_options)

add_subdirectory(src)

if (BUILD_TESTING)
    add_subdirectory(test)
endif()