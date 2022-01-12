add_program(
    TARGET ${LIBRARY_NAME}
    INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}
    SRCS
    ShutdownHandler.cpp
    UnsupportedOperationException.cpp
    InvalidCommandLineOption.cpp
)

add_subdirectory(CommandLine)
add_subdirectory(Event)