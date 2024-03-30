target_sources(
    ${PROJECT_NAME}
    PRIVATE CoreParser.cpp
            Event/Key.cpp
            Event/MouseClick.cpp
            Event/MouseMove.cpp
            Event/MouseScroll.cpp
        Event/Data.cpp
            UnsupportedOperationException.cpp
        Storage/JsonStorage.cpp
        Storage/DatabaseStorage.cpp
        Event/Entry.cpp
        Storage/DatabaseManager.cpp
)
