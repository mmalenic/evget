add_program(
    TARGET ${EVGETCORE_LIBRARY_NAME}
    SRCS
    CoreParser.cpp
    Event/Key.cpp
    Event/MouseClick.cpp
    Event/MouseMove.cpp
    Event/MouseScroll.cpp
    Event/Modifier.cpp
    PrintEvents.cpp
    UnsupportedOperationException.cpp
)