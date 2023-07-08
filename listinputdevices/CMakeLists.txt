cmake_minimum_required(VERSION 3.19)
set(CMAKE_CXX_STANDARD 20)
include("${CMAKE_SOURCE_DIR}/utils.cmake")
project(listinputdevices)

add_library(${PROJECT_NAME} include/listinputdevices/InputDevice.h include/listinputdevices/InputDeviceLister.h)
target_include_directories(${PROJECT_NAME} PUBLIC include)

add_subdirectory(src)

target_link_libraries(${PROJECT_NAME} PUBLIC clioption)

program_dependencies(${PROJECT_NAME} fmt VISIBILITY PUBLIC)
program_dependencies(${PROJECT_NAME} spdlog VISIBILITY PUBLIC)
program_dependencies(${PROJECT_NAME} Boost COMPONENTS headers VISIBILITY PUBLIC)

if(BUILD_TESTING)
    add_subdirectory(test)
endif()
