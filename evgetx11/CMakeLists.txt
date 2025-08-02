set(LIBRARY_NAME libevgetx11)
set(EVGETX11_LIBRARY_NAME
    "${LIBRARY_NAME}"
    PARENT_SCOPE
)

set(INCLUDE include/evgetx11)
set(SRC src)

add_library(
    ${LIBRARY_NAME}
    ${INCLUDE}/DeleterWithDisplay.h
    ${INCLUDE}/EventLoopX11.h
    ${INCLUDE}/EventTransformerX11.h
    ${INCLUDE}/XEventSwitch.h
    ${INCLUDE}/XEventSwitchPointerKey.h
    ${INCLUDE}/XInputEvent.h
    ${INCLUDE}/XInputHandler.h
    ${INCLUDE}/XWrapper.h
    ${INCLUDE}/XWrapperX11.h
    ${INCLUDE}/XEventSwitchTouch.h
)
target_include_directories(${LIBRARY_NAME} PUBLIC include)

target_sources(
    ${LIBRARY_NAME}
    PRIVATE ${SRC}/EventLoopX11.cpp
            ${SRC}/XEventSwitch.cpp
            ${SRC}/XEventSwitchPointerKey.cpp
            ${SRC}/XInputEvent.cpp
            ${SRC}/XInputHandler.cpp
            ${SRC}/XWrapperX11.cpp
            ${SRC}/XEventSwitchTouch.cpp
)

set_property(TARGET ${LIBRARY_NAME} PROPERTY OUTPUT_NAME evgetx11)

target_link_libraries(${LIBRARY_NAME} PUBLIC libevget)

toolbelt_add_dep(${LIBRARY_NAME} spdlog VISIBILITY PUBLIC LINK_COMPONENTS spdlog::spdlog)
toolbelt_add_dep(${LIBRARY_NAME} Boost VISIBILITY PUBLIC LINK_COMPONENTS Boost::headers)
toolbelt_add_dep(
    ${LIBRARY_NAME}
    X11
    VISIBILITY
    PUBLIC
    FIND_PACKAGE_ARGS
    COMPONENTS
    Xi
    Xkb
    LINK_COMPONENTS
    X11::Xi
    X11::Xkb
)

if(BUILD_TESTING)
    target_sources(
        ${TEST_EXECUTABLE_NAME}
        PUBLIC test/EventTransformerX11Test.cpp test/XEventSwitchPointerKeyTest.cpp test/XEventSwitchTest.cpp
               test/XEventSwitchTouchTest.cpp test/utils/Evgetx11TestUtils.cpp test/utils/EvgetX11TestUtils.h
    )
    target_include_directories(${TEST_EXECUTABLE_NAME} PUBLIC test)
endif()

if(EVGET_INSTALL_LIB)
    install(TARGETS ${LIBRARY_NAME} FILE_SET headers)
endif()
