add_program(
    TARGET ${EVGETCORE_LIBRARY_NAME}
    SRCS
    Event/Data.cpp
    CoreParser.cpp
    Event/Key.cpp
    Event/MouseClick.cpp
    Event/MouseMove.cpp
    Event/MouseScroll.cpp
    Event/Schema.cpp
    UnsupportedOperationException.cpp
)