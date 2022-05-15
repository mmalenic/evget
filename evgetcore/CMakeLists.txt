set_and_get_variable(EVGETCORE_LIBRARY_NAME libevgetcore)
set_and_get_variable(EVGETCORE_TEST_EXECUTABLE_NAME libevgetcoretest)

add_program(
    TARGET ${EVGETCORE_LIBRARY_NAME} MAKE_LIBRARY
    INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    SRCS
    include/evgetcore/Task.h
    include/evgetcore/EventTransformer.h
    include/evgetcore/Event/AbstractData.h
    include/evgetcore/EventHandler.h
    include/evgetcore/SystemEvent.h
    include/evgetcore/SystemEventLoop.h
    include/evgetcore/Storage.h
    include/evgetcore/Event/Data.h
    include/evgetcore/Event/Field.h
    include/evgetcore/Event/MouseMove.h
    include/evgetcore/Event/MouseClick.h
    include/evgetcore/Event/MouseScroll.h
    include/evgetcore/Event/Button/Character.h
    include/evgetcore/Event/Pointer/ScrollUp.h
    include/evgetcore/Event/Pointer/ScrollDown.h
    include/evgetcore/Event/Pointer/ScrollLeft.h
    include/evgetcore/Event/Pointer/ScrollRight.h
    include/evgetcore/Event/Common/Time.h
    include/evgetcore/Event/Pointer/Multitouch.h
    include/evgetcore/Event/Pointer/Orientation.h
    include/evgetcore/Event/Pointer/EllipseMajorInner.h
    include/evgetcore/Event/Pointer/EllipseMajorOuter.h
    include/evgetcore/Event/Pointer/EllipseMinorInner.h
    include/evgetcore/Event/Pointer/EllipseMinorOuter.h
    include/evgetcore/Event/Pointer/PositionXOuterTest.h
    include/evgetcore/Event/Pointer/PositionYOuterTest.h
    include/evgetcore/Event/Pointer/PositionX.h
    include/evgetcore/Event/Pointer/PositionY.h
    include/evgetcore/Event/Button/Name.h
    include/evgetcore/Event/Common/DeviceType.h
    include/evgetcore/Event/Common/Device.h
    include/evgetcore/Event/Button/Action.h
    include/evgetcore/Event/Button/ButtonAction.h
    include/evgetcore/Event/Button/Identifier.h
    include/evgetcore/Event/TableData.h
    include/evgetcore/Event/Touch.h
    include/evgetcore/EventListener.h
    include/evgetcore/Util.h
    include/evgetcore/Event/AbstractField.h
    include/evgetcore/UnsupportedOperationException.h
    include/evgetcore/CoreParser.h
)
add_subdirectory(src)

program_dependencies(TARGET ${EVGETCORE_LIBRARY_NAME} DEP PUBLIC fmt ${FMT_VERSION})
program_dependencies(TARGET ${EVGETCORE_LIBRARY_NAME} DEP PUBLIC spdlog ${SPDLOG_VERSION})
program_dependencies(TARGET ${EVGETCORE_LIBRARY_NAME} DEP PUBLIC Boost ${BOOST_VERSION} FIND_ALL COMPONENTS program_options headers)

add_library_and_link(EVGETCORE_LIBRARY_NAME CLIOPTION_LIBRARY_NAME "")

if (BUILD_TESTING)
    add_subdirectory(test)
endif()