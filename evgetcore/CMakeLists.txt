set_and_get_variable(EVGETCORE_LIBRARY_NAME libevgetcore)
set_and_get_variable(EVGETCORE_TEST_EXECUTABLE_NAME libevgetcoretest)

add_program(
    TARGET ${EVGETCORE_LIBRARY_NAME} MAKE_LIBRARY
    INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    SRCS
    include/evgetcore/Task.h
    include/evgetcore/EventTransformer.h
    include/evgetcore/EventHandler.h
    include/evgetcore/SystemEvent.h
    include/evgetcore/SystemEventLoop.h
    include/evgetcore/Storage.h
    include/evgetcore/Event/Data.h
    include/evgetcore/Event/MouseMove.h
    include/evgetcore/Event/MouseClick.h
    include/evgetcore/Event/MouseScroll.h
    include/evgetcore/Event/Direction.h
    include/evgetcore/Event/ButtonAction.h
    include/evgetcore/Event/Device.h
    include/evgetcore/EventListener.h
    include/evgetcore/Util.h
    include/evgetcore/UnsupportedOperationException.h
    include/evgetcore/CoreParser.h
    include/evgetcore/Event/ModifierValue.h
    include/evgetcore/Event/Schema.h
    include/evgetcore/Event/DataType.h
    include/evgetcore/Event/SchemaField.h
    include/evgetcore/Event/Modifier.h
    include/evgetcore/Event/Relation.h
    include/evgetcore/Event/DataWithSchema.h
)
add_subdirectory(src)

program_dependencies(TARGET ${EVGETCORE_LIBRARY_NAME} DEP PUBLIC fmt ${FMT_VERSION})
program_dependencies(TARGET ${EVGETCORE_LIBRARY_NAME} DEP PUBLIC spdlog ${SPDLOG_VERSION})
program_dependencies(TARGET ${EVGETCORE_LIBRARY_NAME} DEP PUBLIC Boost ${BOOST_VERSION} FIND_ALL COMPONENTS program_options headers)
program_dependencies(TARGET ${EVGETCORE_LIBRARY_NAME} DEP PUBLIC date ${DATE_VERSION} COMPONENTS date-tz)

add_library_and_link(EVGETCORE_LIBRARY_NAME CLIOPTION_LIBRARY_NAME "")

if (BUILD_TESTING)
    add_subdirectory(test)
endif ()