add_program(
    TARGET ${EVGETCORE_LIBRARY_NAME}
    SRCS
    Event/Data.cpp
    Event/Field.cpp
    Event/TableData.cpp
    CoreParser.cpp
    Event/Key.cpp
    Event/MouseClick.cpp
    Event/MouseMove.cpp
    Event/MouseScroll.cpp
    UnsupportedOperationException.cpp
)