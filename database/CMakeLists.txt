cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 23)
include("${CMAKE_SOURCE_DIR}/utils.cmake")
project(database)

add_library(
        ${PROJECT_NAME}
        include/database/Migrate.h
        include/database/sqlite/SQLiteConnection.h
        include/database/Connection.h
        include/database/Error.h
        include/database/query/Row.h
        include/database/query/Field.h
        include/database/query/Builder.h
        include/database/query/RowIterator.h
)
target_include_directories(${PROJECT_NAME} PUBLIC include)

add_subdirectory(src)

target_link_libraries(${PROJECT_NAME} PUBLIC util)

program_dependencies(${PROJECT_NAME} spdlog VISIBILITY PUBLIC)
program_dependencies(${PROJECT_NAME} SQLiteCpp VISIBILITY PUBLIC DIRECT_LINK)

#if(BUILD_TESTING)
#    add_subdirectory(test)
#endif()