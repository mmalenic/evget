set(TEST_EXECUTABLE_NAME evgetx11test)

add_executable(
    ${TEST_EXECUTABLE_NAME} EventTransformerX11Test.cpp XEventSwitchTest.cpp
        XEventSwitchPointerKeyTest.cpp utils/EvgetX11TestUtils.h utils/Evgetx11TestUtils.cpp
)
target_include_directories(${TEST_EXECUTABLE_NAME} PUBLIC utils)

if(EVGETX11_HAS_TOUCH_SUPPORT)
    target_sources(${TEST_EXECUTABLE_NAME} PRIVATE XEventSwitchTouchTest.cpp)
endif()

setup_testing(${TEST_EXECUTABLE_NAME} ${PROJECT_NAME})
