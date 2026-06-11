cmake_minimum_required(VERSION 3.30)
set(CMAKE_CXX_STANDARD 23)

# x-release-please-start-version
project(
    evget
    VERSION 0.1.0
    DESCRIPTION "record input device events"
    HOMEPAGE_URL "https://github.com/mmalenic/evget"
)
# x-release-please-end

if(NOT DEFINED toolbelt_SOURCE_DIR)
    include(FetchContent)
    FetchContent_Declare(
        toolbelt
        GIT_REPOSITORY https://github.com/mmalenic/cmake-toolbelt
        GIT_TAG v0.3.3
    )
    FetchContent_MakeAvailable(toolbelt)
endif()
list(APPEND CMAKE_MODULE_PATH "${toolbelt_SOURCE_DIR}/src")

include(toolbelt)
include(cmake/windows.cmake)
include(cmake/warnings.cmake)

if(BUILD_TESTING)
    enable_testing()
    set(TEST_EXECUTABLE_NAME evgettest)

    add_executable(${TEST_EXECUTABLE_NAME} evget/test/main.cpp)
endif()

add_subdirectory(evget)

if(EVGET_BUILD_EVGETX11)
    add_subdirectory(evgetx11)
endif()
if(EVGET_BUILD_EVGETLIBINPUT)
    add_subdirectory(evgetlibinput)
endif()
if(EVGET_BUILD_EVGETWINDOWS)
    add_subdirectory(evgetwindows)
endif()

if(BUILD_TESTING)
    list(APPEND LIBRARIES ${EVGET_LIBRARY_NAME})
    if(EVGET_BUILD_EVGETX11)
        list(APPEND LIBRARIES ${EVGETX11_LIBRARY_NAME})
    endif()
    if(EVGET_BUILD_EVGETLIBINPUT)
        list(APPEND LIBRARIES ${EVGETLIBINPUT_LIBRARY_NAME})
    endif()
    if(EVGET_BUILD_EVGETWINDOWS)
        list(APPEND LIBRARIES ${EVGETWINDOWS_LIBRARY_NAME})
    endif()

    toolbelt_setup_gtest(${TEST_EXECUTABLE_NAME} ADD_LIBRARIES ${LIBRARIES})

    evget_copy_runtime_dlls(${TEST_EXECUTABLE_NAME})
endif()

if(EVGET_BUILD_BIN)
    # Check if at least one feature has been built
    if(NOT EVGET_BUILD_EVGETX11
       AND NOT EVGET_BUILD_EVGETLIBINPUT
       AND NOT EVGET_BUILD_EVGETWINDOWS
    )
        message(FATAL_ERROR "evget: at least one feature should be built - evgetx11, evgetlibinput, evgetwindows")
    endif()

    add_executable(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/evget/src/main.cpp)
    set_property(TARGET ${PROJECT_NAME} PROPERTY RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

    target_link_libraries(${PROJECT_NAME} PRIVATE ${EVGET_LIBRARY_NAME})

    if(EVGET_BUILD_EVGETX11)
        target_compile_definitions(${PROJECT_NAME} PRIVATE FEATURE_EVGETX11=1)
        target_link_libraries(${PROJECT_NAME} PRIVATE ${EVGETX11_LIBRARY_NAME})
    endif()
    if(EVGET_BUILD_EVGETLIBINPUT)
        target_compile_definitions(${PROJECT_NAME} PRIVATE FEATURE_EVGETLIBINPUT=1)
        target_link_libraries(${PROJECT_NAME} PRIVATE ${EVGETLIBINPUT_LIBRARY_NAME})
    endif()
    if(EVGET_BUILD_EVGETWINDOWS)
        target_compile_definitions(${PROJECT_NAME} PRIVATE FEATURE_EVGETWINDOWS=1)
        target_link_libraries(${PROJECT_NAME} PRIVATE ${EVGETWINDOWS_LIBRARY_NAME})
    endif()

    evget_copy_runtime_dlls(${PROJECT_NAME})
endif()

if(EVGET_INSTALL_BIN AND EVGET_BUILD_BIN)
    install(TARGETS ${PROJECT_NAME})
endif()

if(CMAKE_EXPORT_COMPILE_COMMANDS AND CMAKE_GENERATOR MATCHES "Ninja|Makefiles")
    add_custom_target(
        copy_compile_commands ALL
        ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/compile_commands.json ${CMAKE_SOURCE_DIR}
        COMMENT "copying compile_commands.json to project root"
    )
endif()

foreach(target ${EVGET_LIBRARY_NAME} ${PROJECT_NAME} ${TEST_EXECUTABLE_NAME} ${EVGETX11_LIBRARY_NAME}
               ${EVGETLIBINPUT_LIBRARY_NAME} ${EVGETWINDOWS_LIBRARY_NAME}
)
    if(TARGET ${target})
        evget_apply_warnings(${target})
        if(WIN32)
            evget_apply_windows_defines(${target})
        endif()
    endif()
endforeach()
