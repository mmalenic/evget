cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 23)
include("${CMAKE_SOURCE_DIR}/utils.cmake")
project(evgetcore)

add_library(
    ${PROJECT_NAME}
    include/evgetcore/EventTransformer.h
    include/evgetcore/EventHandler.h
        include/evgetcore/Storage/Store.h
    include/evgetcore/EventLoop.h
        include/evgetcore/Event/MouseMove.h
    include/evgetcore/Event/MouseClick.h
    include/evgetcore/Event/MouseScroll.h
    include/evgetcore/Event/Direction.h
    include/evgetcore/Event/ButtonAction.h
    include/evgetcore/Event/Device.h
    include/evgetcore/EventListener.h
    include/evgetcore/UnsupportedOperationException.h
        include/evgetcore/Parser.h
    include/evgetcore/Event/ModifierValue.h
    include/evgetcore/Event/Schema.h
    include/evgetcore/Event/Graph.h
        include/evgetcore/Storage/JsonStorage.h
        include/evgetcore/Storage/DatabaseStorage.h
        include/evgetcore/Storage/Error.h
        include/evgetcore/Event/Data.h
        include/evgetcore/Event/Entry.h
        include/evgetcore/Storage/DatabaseManager.h
)
target_include_directories(${PROJECT_NAME} PUBLIC include)

add_subdirectory(src)
add_subdirectory(database)

target_link_libraries(${PROJECT_NAME} PUBLIC util)
target_link_libraries(${PROJECT_NAME} PUBLIC async)
target_link_libraries(${PROJECT_NAME} PUBLIC clioption)
target_link_libraries(${PROJECT_NAME} PUBLIC database)

program_dependencies(${PROJECT_NAME} fmt VISIBILITY PUBLIC)
program_dependencies(${PROJECT_NAME} spdlog VISIBILITY PUBLIC)
program_dependencies(
    ${PROJECT_NAME}
    Boost
    COMPONENTS
    program_options
    VISIBILITY
    PUBLIC
    LINK_COMPONENTS
    program_options
    headers
)
program_dependencies(${PROJECT_NAME} date COMPONENTS date-tz VISIBILITY PUBLIC)
program_dependencies(${PROJECT_NAME} nlohmann_json VISIBILITY PUBLIC)
program_dependencies(${PROJECT_NAME} SQLiteCpp VISIBILITY PUBLIC DIRECT_LINK)

if(BUILD_TESTING)
    add_subdirectory(test)
endif()
