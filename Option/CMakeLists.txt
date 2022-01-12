set(OPTION_LIBRARY_NAME liboption PARENT_SCOPE)
set(OPTION_TEST_NAME optiontest PARENT_SCOPE)

add_program(
    TARGET ${OPTION_LIBRARY_NAME} MAKE_LIBRARY
    INCLUDE_DIRS
    include
    SRCS
    include/Option/Option.h
    include/Option/OptionBase.h
    include/Option/OptionBuilder.h
    include/Option/OptionFlag.h
    include/Option/OptionValidated.h
)

add_subdirectory(src)
add_subdirectory(test)