add_program(
    TARGET ${EVGETCORE_LIBRARY_NAME}
    SRCS
    Event/Data.cpp
    Event/Field.cpp
    Event/TableData.cpp
    Event/AbstractData.cpp
    CoreParser.cpp
    Event/Key.cpp
    Event/MouseClick.cpp
    Event/MouseMove.cpp
    Event/MouseScroll.cpp
    Event/Touch.cpp
    UnsupportedOperationException.cpp
)