set(LIBRARY_NAME libevgetx11)
set(EVGETX11_LIBRARY_NAME "${LIBRARY_NAME}" PARENT_SCOPE)

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
        ${INCLUDE}/XSetMask.h
        ${INCLUDE}/XSetMaskCore.h
        ${INCLUDE}/XSetMaskRefresh.h
        ${INCLUDE}/XWrapper.h
        ${INCLUDE}/XWrapperX11.h
)
target_include_directories(${LIBRARY_NAME} PUBLIC include)

target_sources(
        ${LIBRARY_NAME}
        PRIVATE
        ${SRC}/EventLoopX11.cpp
        ${SRC}/XEventSwitch.cpp
        ${SRC}/XEventSwitchPointerKey.cpp
        ${SRC}/XInputEvent.cpp
        ${SRC}/XInputHandler.cpp
        ${SRC}/XSetMaskCore.cpp
        ${SRC}/XSetMaskRefresh.cpp
        ${SRC}/XWrapperX11.cpp
)

toolbelt_check_symbol(VAR EVGETX11_HAS_TOUCH_SUPPORT SYMBOL XI_TouchBegin FILES X11/extensions/XInput2.h)
if(EVGETX11_HAS_TOUCH_SUPPORT)
    target_sources(${LIBRARY_NAME}
            PUBLIC
            ${INCLUDE}/XEventSwitchTouch.h
            ${INCLUDE}/XSetMaskTouch.h
    )
    target_sources(${LIBRARY_NAME}
            PRIVATE
            ${SRC}/XEventSwitchTouch.cpp
            ${SRC}/XSetMaskTouch.cpp
    )
endif()

set_property(TARGET ${LIBRARY_NAME} PROPERTY OUTPUT_NAME evgetx11)

target_link_libraries(${LIBRARY_NAME} PUBLIC libevget)

toolbelt_add_dep(${LIBRARY_NAME} spdlog VISIBILITY PUBLIC LINK_COMPONENTS spdlog::spdlog)
toolbelt_add_dep(
        ${LIBRARY_NAME}
        Boost
        VISIBILITY
        PUBLIC
        LINK_COMPONENTS
        Boost::headers
)
toolbelt_add_dep(
        ${LIBRARY_NAME}
        X11
        VISIBILITY
        PUBLIC
        FIND_PACKAGE_ARGS COMPONENTS Xi Xkb
        LINK_COMPONENTS X11::Xi X11::Xkb
)

if(BUILD_TESTING)
    set(TEST test)

    target_sources(
            ${TEST_EXECUTABLE_NAME}
            PUBLIC
            ${TEST}/EventTransformerX11Test.cpp
            ${TEST}/XEventSwitchPointerKeyTest.cpp
            ${TEST}/XEventSwitchTest.cpp
            ${TEST}/XEventSwitchTouchTest.cpp
            ${TEST}/utils/Evgetx11TestUtils.cpp
            ${TEST}/utils/EvgetX11TestUtils.h
    )
    target_include_directories(${TEST_EXECUTABLE_NAME} PUBLIC test)
endif()

if(EVGET_INSTALL_LIB)
    install(TARGETS ${LIBRARY_NAME} FILE_SET headers)
endif()