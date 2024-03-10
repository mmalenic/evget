cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 23)
include("${CMAKE_SOURCE_DIR}/utils.cmake")
project(database)

add_library(
        ${PROJECT_NAME}
        include/database/Migrate.h
        include/database/sqlite/Connection.h
        include/database/Connection.h
        include/database/Error.h
        include/database/Query.h
        include/database/sqlite/Query.h
)
target_include_directories(${PROJECT_NAME} PUBLIC include)

add_subdirectory(src)

target_link_libraries(${PROJECT_NAME} PUBLIC util)

program_dependencies(${PROJECT_NAME} spdlog VISIBILITY PUBLIC)
program_dependencies(${PROJECT_NAME} cryptopp VISIBILITY PUBLIC)
program_dependencies(${PROJECT_NAME} SQLiteCpp VISIBILITY PUBLIC DIRECT_LINK)

if(BUILD_TESTING)
    program_dependencies(${PROJECT_NAME}
        Boost
        VISIBILITY
        PUBLIC
        LINK_COMPONENTS
        headers
    )
    add_subdirectory(test)
endif()