add_program(
    TARGET ${TEST_EXECUTABLE_NAME}
    SRCS
    CommandLine/CommandLineOptionBaseTest.cpp
    CommandLine/CommandLineOptionFlagTest.cpp
    CommandLine/CommandLineOptionTest.cpp
    CommandLine/CommandLineOptionValidatedTest.cpp
    CommandLine/CommandLineParserTest.cpp
    Event/Common/TimeTest.cpp
    Event/Button/CharacterTest.cpp
    Event/Button/PressTest.cpp
    Event/Button/ReleaseTest.cpp
    Event/Button/RepeatTest.cpp
    Event/Cursor/PositionXTest.cpp
    Event/Cursor/PositionYTest.cpp
    Event/Cursor/WheelDown.cpp
    Event/Cursor/WheelUp.cpp
    Event/Touch/EllipseMajorInnerTest.cpp
    Event/Touch/EllipseMajorOuterTest.cpp
    Event/Touch/EllipseMinorInnerTest.cpp
    Event/Touch/EllipseMinorOuterTest.cpp
    Event/Touch/MultitouchTest.cpp
    Event/Touch/OrientationTest.cpp
    Event/EventDataTest.cpp
    Event/FieldTest.cpp
)

add_subdirectory(TestUtils)