if (BUILD_TESTING)
    add_program(
        TARGET ${OPTION_TEST_NAME} MAKE_EXECUTABLE
        SRCS
        CommandLineOptionBaseTest.cpp
        CommandLineOptionFlagTest.cpp
        CommandLineOptionTest.cpp
        CommandLineOptionValidatedTest.cpp
    )
endif()