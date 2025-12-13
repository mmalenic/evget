set(LIBRARY_NAME libevgetlibinput)
set(EVGETLIBINPUT_LIBRARY_NAME
    "${LIBRARY_NAME}"
    PARENT_SCOPE
)

set(INCLUDE include/evgetlibinput)
set(SRC src)

add_library(${LIBRARY_NAME})
target_sources(
    ${LIBRARY_NAME}
    PRIVATE # ${SRC}/event_loop.cpp ${SRC}/event_switch.cpp ${SRC}/event_switch_pointer_key.cpp
            # ${SRC}/event_switch_touch.cpp ${SRC}/event_transformer.cpp ${SRC}/input_event.cpp ${SRC}/input_handler.cpp
            ${SRC}/libinput_api.cpp
            # ${SRC}/udev_context.cpp
    PUBLIC FILE_SET
           HEADERS
           BASE_DIRS
           include
           FILES
           # ${INCLUDE}/event_handler.h ${INCLUDE}/event_loop.h ${INCLUDE}/event_switch.h
           # ${INCLUDE}/event_switch_pointer_key.h ${INCLUDE}/event_switch_touch.h ${INCLUDE}/event_transformer.h
           # ${INCLUDE}/input_event.h ${INCLUDE}/input_handler.h
           ${INCLUDE}/libinput_api.h
    # ${INCLUDE}/udev_context.h
)
set_property(TARGET ${LIBRARY_NAME} PROPERTY OUTPUT_NAME evgetlibinput)

target_link_libraries(${LIBRARY_NAME} PUBLIC libevget)

toolbelt_add_dep(
    ${LIBRARY_NAME}
    spdlog
    VISIBILITY
    PUBLIC
    LINK_COMPONENTS
    spdlog::spdlog
    FIND_PACKAGE_ARGS
    REQUIRED
)
toolbelt_add_dep(
    ${LIBRARY_NAME}
    Boost
    VISIBILITY
    PUBLIC
    LINK_COMPONENTS
    Boost::headers
    FIND_PACKAGE_ARGS
    REQUIRED
)

# Try conan-specific install first, otherwise fallback to system install.
toolbelt_add_dep(
    ${LIBRARY_NAME}
    libinput
    VISIBILITY
    PUBLIC
    LINK_COMPONENTS
    libinput::libinput
    libudev::libudev
)
if(NOT TARGET libinput::libinput)
    toolbelt_add_dep(
        ${LIBRARY_NAME}
        Libinput
        VISIBILITY
        PUBLIC
        LINK_COMPONENTS
        Libinput::Libinput
        FIND_PACKAGE_ARGS
        REQUIRED
    )
endif()

# if(BUILD_TESTING) target_sources( ${TEST_EXECUTABLE_NAME} PUBLIC test/common/libinput_api_mock.cpp
# test/common/libinput_api_mock.h test/event_switch.cpp test/event_switch_pointer_key.cpp test/event_switch_touch.cpp
# test/event_transformer.cpp ) target_include_directories(${TEST_EXECUTABLE_NAME} PUBLIC test) endif()

if(EVGET_INSTALL_LIB)
    install(TARGETS ${LIBRARY_NAME} FILE_SET headers)
endif()
