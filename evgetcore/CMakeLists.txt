set(LIBRARY_NAME libevget)
set(EVGET_LIBRARY_NAME
    "${LIBRARY_NAME}"
    PARENT_SCOPE
)

set(INCLUDE include/evgetcore)
set(SRC src)

add_library(
    ${LIBRARY_NAME}
    ${INCLUDE}/database/sqlite/Connection.h
    ${INCLUDE}/database/sqlite/Query.h
    ${INCLUDE}/database/Connection.h
    ${INCLUDE}/database/Migrate.h
    ${INCLUDE}/database/Query.h
    ${INCLUDE}/EventTransformer.h
    ${INCLUDE}/EventHandler.h
    ${INCLUDE}/Storage/Store.h
    ${INCLUDE}/EventLoop.h
    ${INCLUDE}/Event/MouseMove.h
    ${INCLUDE}/Event/MouseClick.h
    ${INCLUDE}/Event/MouseScroll.h
    ${INCLUDE}/Event/Direction.h
    ${INCLUDE}/Event/ButtonAction.h
    ${INCLUDE}/Event/Device.h
    ${INCLUDE}/EventListener.h
    ${INCLUDE}/cli.h
    ${INCLUDE}/Event/ModifierValue.h
    ${INCLUDE}/Event/Schema.h
    ${INCLUDE}/Event/Graph.h
    ${INCLUDE}/Storage/JsonStorage.h
    ${INCLUDE}/Storage/DatabaseStorage.h
    ${INCLUDE}/Event/Data.h
    ${INCLUDE}/Event/Entry.h
    ${INCLUDE}/Storage/DatabaseManager.h
    ${INCLUDE}/Error.h
    ${INCLUDE}/async/container/LockingVector.h
    ${INCLUDE}/async/scheduler/Interval.h
    ${INCLUDE}/async/scheduler/Scheduler.h
)
target_include_directories(${LIBRARY_NAME} PUBLIC include)

target_sources(
    ${LIBRARY_NAME}
    PRIVATE ${SRC}/cli.cpp
            ${SRC}/Event/Key.cpp
            ${SRC}/Event/MouseClick.cpp
            ${SRC}/Event/MouseMove.cpp
            ${SRC}/Event/MouseScroll.cpp
            ${SRC}/Event/Data.cpp
            ${SRC}/Storage/JsonStorage.cpp
            ${SRC}/Storage/DatabaseStorage.cpp
            ${SRC}/Event/Entry.cpp
            ${SRC}/Storage/DatabaseManager.cpp
            ${SRC}/database/Migrate.cpp
            ${SRC}/database/sqlite/Connection.cpp
            ${SRC}/database/sqlite/Query.cpp
            ${SRC}/async/scheduler/Interval.cpp
            ${SRC}/async/scheduler/Scheduler.cpp
)

set_property(TARGET ${LIBRARY_NAME} PROPERTY OUTPUT_NAME evget)

set(SCHEMA database/schema)
set(QUERIES database/queries)
set(SCHEMA_GENERATED schema)
set(QUERIES_GENERATED queries)
set(NAMESPACE EvgetCore::Database::detail)

toolbelt_embed(
    ${SCHEMA_GENERATED}/initialize.h
    initialize
    EMBED
    ${SCHEMA}/000_schema_types.sql
    ${SCHEMA}/001_schema_modifier.sql
    ${SCHEMA}/002_schema_key.sql
    ${SCHEMA}/003_schema_mouse_click.sql
    ${SCHEMA}/004_schema_mouse_move.sql
    ${SCHEMA}/005_schema_mouse_scroll.sql
    NAMESPACE
    ${NAMESPACE}
    TARGET
    ${LIBRARY_NAME}
)
toolbelt_embed(
    ${QUERIES_GENERATED}/insert_key.h
    insert_key
    NAMESPACE
    ${NAMESPACE}
    EMBED
    ${QUERIES}/insert_key.sql
    TARGET
    ${LIBRARY_NAME}
)
toolbelt_embed(
    ${QUERIES_GENERATED}/insert_key_modifier.h
    insert_key_modifier
    NAMESPACE
    ${NAMESPACE}
    EMBED
    ${QUERIES}/insert_key_modifier.sql
    TARGET
    ${LIBRARY_NAME}
)
toolbelt_embed(
    ${QUERIES_GENERATED}/insert_mouse_click.h
    insert_mouse_click
    NAMESPACE
    ${NAMESPACE}
    EMBED
    ${QUERIES}/insert_mouse_click.sql
    TARGET
    ${LIBRARY_NAME}
)
toolbelt_embed(
    ${QUERIES_GENERATED}/insert_mouse_click_modifier.h
    insert_mouse_click_modifier
    NAMESPACE
    ${NAMESPACE}
    EMBED
    ${QUERIES}/insert_mouse_click_modifier.sql
    TARGET
    ${LIBRARY_NAME}
)
toolbelt_embed(
    ${QUERIES_GENERATED}/insert_mouse_move.h
    insert_mouse_move
    NAMESPACE
    ${NAMESPACE}
    EMBED
    ${QUERIES}/insert_mouse_move.sql
    TARGET
    ${LIBRARY_NAME}
)
toolbelt_embed(
    ${QUERIES_GENERATED}/insert_mouse_move_modifier.h
    insert_mouse_move_modifier
    NAMESPACE
    ${NAMESPACE}
    EMBED
    ${QUERIES}/insert_mouse_move_modifier.sql
    TARGET
    ${LIBRARY_NAME}
)
toolbelt_embed(
    ${QUERIES_GENERATED}/insert_mouse_scroll.h
    insert_mouse_scroll
    NAMESPACE
    ${NAMESPACE}
    EMBED
    ${QUERIES}/insert_mouse_scroll.sql
    TARGET
    ${LIBRARY_NAME}
)
toolbelt_embed(
    ${QUERIES_GENERATED}/insert_mouse_scroll_modifier.h
    insert_mouse_scroll_modifier
    NAMESPACE
    ${NAMESPACE}
    EMBED
    ${QUERIES}/insert_mouse_scroll_modifier.sql
    TARGET
    ${LIBRARY_NAME}
)
target_include_directories(${LIBRARY_NAME} PRIVATE ${cmake_toolbelt_ret})

toolbelt_add_dep(${LIBRARY_NAME} spdlog VISIBILITY PUBLIC LINK_COMPONENTS spdlog::spdlog)
toolbelt_add_dep(${LIBRARY_NAME} Boost VISIBILITY PUBLIC LINK_COMPONENTS Boost::headers)
toolbelt_add_dep(${LIBRARY_NAME} date VISIBILITY PUBLIC LINK_COMPONENTS date::date)
toolbelt_add_dep(${LIBRARY_NAME} nlohmann_json VISIBILITY PUBLIC LINK_COMPONENTS nlohmann_json::nlohmann_json)
toolbelt_add_dep(${LIBRARY_NAME} SQLiteCpp VISIBILITY PUBLIC LINK_COMPONENTS SQLiteCpp)
toolbelt_add_dep(${LIBRARY_NAME} openssl VISIBILITY PUBLIC LINK_COMPONENTS openssl::openssl)
toolbelt_add_dep(${LIBRARY_NAME} CLI11 VISIBILITY PUBLIC LINK_COMPONENTS CLI11::CLI11)

file(STRINGS "../LICENSE" LICENSE)
list(GET LICENSE 0 LICENSE_NAME)
list(GET LICENSE 2 LICENSE_COPYRIGHT)
target_compile_definitions(${LIBRARY_NAME} PUBLIC EVGET_LICENSE="${LICENSE_NAME}")
target_compile_definitions(${LIBRARY_NAME} PUBLIC EVGET_COPYRIGHT="${LICENSE_COPYRIGHT}")
# x-release-please-start-version
target_compile_definitions(${LIBRARY_NAME} PUBLIC EVGET_VERSION="0.1.0")
# x-release-please-end

if(BUILD_TESTING)
    enable_testing()
    set(TEST test)

    target_sources(
        ${TEST_EXECUTABLE_NAME}
        PUBLIC ${TEST}/async/container/LockingVector.cpp
               ${TEST}/async/scheduler/Interval.cpp
               ${TEST}/async/scheduler/Scheduler.cpp
               ${TEST}/database/sqlite/Connection.cpp
               ${TEST}/database/sqlite/Migrate.cpp
               ${TEST}/database/sqlite/Query.cpp
               ${TEST}/Event/MouseMoveTest.cpp
               ${TEST}/Event/MouseClickTest.cpp
               ${TEST}/Event/KeyTest.cpp
               ${TEST}/Event/MouseScrollTest.cpp
               ${TEST}/Event/DataTest.cpp
               ${TEST}/utils/DatabaseTest.h
               ${TEST}/utils/DatabaseTest.cpp
    )
    target_include_directories(${TEST_EXECUTABLE_NAME} PUBLIC test)
endif()

if(EVLIST_INSTALL_LIB)
    install(TARGETS ${LIBRARY_NAME} FILE_SET headers)
endif()
