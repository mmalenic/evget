cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 23)
include("${CMAKE_SOURCE_DIR}/utils.cmake")
project(listinputdevices)

add_library(${PROJECT_NAME} include/listinputdevices/InputDevice.h include/listinputdevices/InputDeviceLister.h)
target_include_directories(${PROJECT_NAME} PUBLIC include)

add_subdirectory(src)

target_link_libraries(${PROJECT_NAME} PUBLIC clioption)

program_dependencies(${PROJECT_NAME} fmt VISIBILITY PUBLIC)
program_dependencies(${PROJECT_NAME} spdlog VISIBILITY PUBLIC)
program_dependencies(${PROJECT_NAME} Boost VISIBILITY PUBLIC LINK_COMPONENTS headers)

if(BUILD_TESTING)
    add_subdirectory(test)
endif()
