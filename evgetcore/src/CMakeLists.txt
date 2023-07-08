target_sources(
    ${PROJECT_NAME}
    PRIVATE main.cpp
            CoreParser.cpp
            Event/Key.cpp
            Event/MouseClick.cpp
            Event/MouseMove.cpp
            Event/MouseScroll.cpp
            Event/Modifier.cpp
            PrintEvents.cpp
            UnsupportedOperationException.cpp
)
