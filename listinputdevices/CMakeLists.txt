set_and_get_variable(LISTINPUTDEVICES_LIBRARY_NAME liblistinputdevices)
set_and_get_variable(LISTINPUTDEVICES_TEST_EXECUTABLE_NAME listinputdevicestest)

add_program(
    TARGET ${LISTINPUTDEVICES_LIBRARY_NAME} MAKE_LIBRARY
    FEATURE_REQUIRES fcntl.h linux/input.h
    FEATURE_CHECK LISTINPUTDEVICES_HAS_DEPENDENCIES
    INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    SRCS
    include/listinputdevices/InputDevice.h
    include/listinputdevices/InputDeviceLister.h
)

add_subdirectory(src)

program_dependencies(TARGET ${LISTINPUTDEVICES_LIBRARY_NAME} DEP PUBLIC fmt ${FMT_VERSION})
program_dependencies(TARGET ${LISTINPUTDEVICES_LIBRARY_NAME} DEP PUBLIC spdlog ${SPDLOG_VERSION})
program_dependencies(TARGET ${LISTINPUTDEVICES_LIBRARY_NAME} DEP PUBLIC Boost ${BOOST_VERSION} FIND_ALL COMPONENTS headers)

add_library_and_link(LISTINPUTDEVICES_LIBRARY_NAME CLIOPTION_LIBRARY_NAME "")

if (BUILD_TESTING)
    add_subdirectory(test)
endif()