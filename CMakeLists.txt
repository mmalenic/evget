cmake_minimum_required(VERSION 3.24)
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

if(EVGET_RUN_CLANG_TIDY)
    if(DEFINED EVGET_CLANG_TIDY_EXECUTABLE)
        set(CMAKE_CXX_CLANG_TIDY "${EVGET_CLANG_TIDY_EXECUTABLE}")
    else()
        set(CMAKE_CXX_CLANG_TIDY "clang-tidy")
    endif()

    if(EVGET_CLANG_TIDY_FIX_ERRORS)
        list(APPEND CMAKE_CXX_CLANG_TIDY "--fix-errors")
    endif()
endif()

if(BUILD_TESTING)
    enable_testing()
    set(TEST_EXECUTABLE_NAME evgettest)

    add_executable(${TEST_EXECUTABLE_NAME} evget/test/main.cpp)
endif()

add_subdirectory(evget)

if(EVGET_BUILD_EVGETX11)
    toolbelt_check_includes(
        VAR
        FEATURE_EVGETX11
        INCLUDES
        X11/Xlib.h
        X11/extensions/XInput.h
        X11/extensions/XInput2.h
        X11/Xutil.h
        xorg/xserver-properties.h
        LANGUAGE
        CXX
    )
    if(FEATURE_EVGETX11)
        add_subdirectory(evgetx11)
    endif()
endif()

if(BUILD_TESTING)
    list(APPEND LIBRARIES ${EVGET_LIBRARY_NAME})
    if(EVGETX11_HAS_FEATURE)
        list(APPEND LIBRARIES ${EVGETX11_LIBRARY_NAME})
    endif()

    toolbelt_setup_gtest(${TEST_EXECUTABLE_NAME} ADD_LIBRARIES ${LIBRARIES})
endif()

if(EVGET_BUILD_BIN)
    # Check if at least one feature has been built
    if(NOT FEATURE_EVGETX11)
        message(FATAL_ERROR "evget: at least one feature should be built - evgetx11")
    endif()

    add_executable(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/evget/src/main.cpp)
    set_property(TARGET ${PROJECT_NAME} PROPERTY RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

    target_link_libraries(${PROJECT_NAME} PRIVATE ${EVGET_LIBRARY_NAME})
    if(FEATURE_EVGETX11)
        target_link_libraries(${PROJECT_NAME} PRIVATE ${EVGETX11_LIBRARY_NAME})
    endif()
endif()

if(EVGET_INSTALL_BIN AND EVGET_BUILD_BIN)
    install(TARGETS ${PROJECT_NAME})
endif()
