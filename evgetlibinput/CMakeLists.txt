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
    PRIVATE ${SRC}/next_event.cpp
            ${SRC}/event_transformer.cpp
            # ${SRC}/event_switch.cpp ${SRC}/event_switch_pointer_key.cpp ${SRC}/event_switch_touch.cpp
            # ${SRC}/event_transformer.cpp ${SRC}/input_event.cpp ${SRC}/input_handler.cpp
            ${SRC}/libinput.cpp
            ${SRC}/drm.cpp
            ${SRC}/xkbcommon.cpp
            # ${SRC}/udev_context.cpp
    PUBLIC FILE_SET
           HEADERS
           BASE_DIRS
           include
           FILES
           ${INCLUDE}/event_transformer.h
           ${INCLUDE}/next_event.h
           # ${INCLUDE}/event_handler.h ${INCLUDE}/event_switch.h ${INCLUDE}/event_switch_pointer_key.h
           # ${INCLUDE}/event_switch_touch.h ${INCLUDE}/event_transformer.h ${INCLUDE}/input_event.h
           # ${INCLUDE}/input_handler.h
           ${INCLUDE}/libinput.h
           ${INCLUDE}/drm.h
           ${INCLUDE}/xkbcommon.h
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

toolbelt_add_dep(
    ${LIBRARY_NAME}
    libudev
    VISIBILITY
    PUBLIC
    LINK_COMPONENTS
    libudev::libudev
    FIND_PACKAGE_ARGS
    REQUIRED
)
toolbelt_add_dep(
    ${LIBRARY_NAME}
    xkbcommon
    VISIBILITY
    PUBLIC
    LINK_COMPONENTS
    xkbcommon::xkbcommon
    FIND_PACKAGE_ARGS
    REQUIRED
)

toolbelt_add_dep(
    ${LIBRARY_NAME}
    wayland
    VISIBILITY
    PUBLIC
    LINK_COMPONENTS
    wayland::wayland-client
    FIND_PACKAGE_ARGS
    REQUIRED
)
toolbelt_add_dep(
    ${LIBRARY_NAME}
    libdrm
    VISIBILITY
    PUBLIC
    LINK_COMPONENTS
    libdrm::libdrm
    FIND_PACKAGE_ARGS
    REQUIRED
)
toolbelt_add_dep(
    ${LIBRARY_NAME}
    libevdev
    VISIBILITY
    PUBLIC
    LINK_COMPONENTS
    libevdev::libevdev
    FIND_PACKAGE_ARGS
    REQUIRED
)

# Use system libinput which conan is able to install.
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

if(BUILD_TESTING)
    target_sources(
        ${TEST_EXECUTABLE_NAME} PUBLIC test/common/test_helpers.cpp test/common/test_helpers.h
                                       test/event_transformer.cpp test/xkbcommon.cpp
    )
    target_include_directories(${TEST_EXECUTABLE_NAME} PUBLIC test)
endif()

if(EVGET_INSTALL_LIB)
    install(TARGETS ${LIBRARY_NAME} FILE_SET headers)
endif()
