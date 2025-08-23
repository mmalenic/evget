set(LIBRARY_NAME libevgetx11)
set(EVGETX11_LIBRARY_NAME
    "${LIBRARY_NAME}"
    PARENT_SCOPE
)

set(INCLUDE include/evgetx11)
set(SRC src)

add_library(${LIBRARY_NAME})
target_sources(
    ${LIBRARY_NAME}
    PRIVATE ${SRC}/event_loop.cpp
            ${SRC}/event_switch.cpp
            ${SRC}/event_switch_pointer_key.cpp
            ${SRC}/event_switch_touch.cpp
            ${SRC}/input_event.cpp
            ${SRC}/input_handler.cpp
            ${SRC}/x11_api.cpp
    PUBLIC FILE_SET
           HEADERS
           BASE_DIRS
           include
           FILES
           ${INCLUDE}/display_deleter.h
           ${INCLUDE}/event_loop.h
           ${INCLUDE}/event_switch.h
           ${INCLUDE}/event_switch_pointer_key.h
           ${INCLUDE}/event_switch_touch.h
           ${INCLUDE}/event_transformer.h
           ${INCLUDE}/input_event.h
           ${INCLUDE}/input_handler.h
           ${INCLUDE}/x11_api.h
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
        ${TEST_EXECUTABLE_NAME} PUBLIC test/common/x11_api_mock.cpp test/common/x11_api_mock.h test/event_switch.cpp
    )
    target_include_directories(${TEST_EXECUTABLE_NAME} PUBLIC test)
endif()

if(EVGET_INSTALL_LIB)
    install(TARGETS ${LIBRARY_NAME} FILE_SET headers)
endif()
