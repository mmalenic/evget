cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 23)
include("${CMAKE_SOURCE_DIR}/utils.cmake")
project(clioption)

add_library(
    ${PROJECT_NAME}
    include/clioption/AbstractOption.h
    include/clioption/InvalidCommandLineOption.h
    include/clioption/Option.h
    include/clioption/OptionBuilder.h
    include/clioption/OptionFlag.h
    include/clioption/OptionValidated.h
    include/clioption/Parser.h
)
target_include_directories(${PROJECT_NAME} PUBLIC include)

add_subdirectory(src)

program_dependencies(${PROJECT_NAME} fmt VISIBILITY PUBLIC)
program_dependencies(${PROJECT_NAME} spdlog VISIBILITY PUBLIC)
program_dependencies(${PROJECT_NAME} Boost COMPONENTS program_options VISIBILITY PUBLIC)

if(BUILD_TESTING)
    add_subdirectory(test)
endif()
