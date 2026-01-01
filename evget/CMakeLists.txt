set(LIBRARY_NAME libevget)
set(EVGET_LIBRARY_NAME
    "${LIBRARY_NAME}"
    PARENT_SCOPE
)

set(INCLUDE include/evget)
set(SRC src)

add_library(${LIBRARY_NAME})
target_sources(
    ${LIBRARY_NAME}
    PRIVATE ${SRC}/cli.cpp
            ${SRC}/event/key.cpp
            ${SRC}/event/mouse_click.cpp
            ${SRC}/event/mouse_move.cpp
            ${SRC}/event/mouse_scroll.cpp
            ${SRC}/event/data.cpp
            ${SRC}/storage/json_storage.cpp
            ${SRC}/storage/database_storage.cpp
            ${SRC}/event/entry.cpp
            ${SRC}/storage/database_manager.cpp
            ${SRC}/database/migrate.cpp
            ${SRC}/database/sqlite/connection.cpp
            ${SRC}/database/sqlite/query.cpp
            ${SRC}/async/scheduler/interval.cpp
            ${SRC}/async/scheduler/scheduler.cpp
    PUBLIC FILE_SET
           HEADERS
           BASE_DIRS
           include
           FILES
           ${INCLUDE}/database/sqlite/connection.h
           ${INCLUDE}/database/sqlite/query.h
           ${INCLUDE}/database/connection.h
           ${INCLUDE}/database/migrate.h
           ${INCLUDE}/database/query.h
           ${INCLUDE}/event_transformer.h
           ${INCLUDE}/event_handler.h
           ${INCLUDE}/input_event.h
           ${INCLUDE}/next_event.h
           ${INCLUDE}/storage/store.h
           ${INCLUDE}/event_loop.h
           ${INCLUDE}/event/mouse_move.h
           ${INCLUDE}/event/mouse_click.h
           ${INCLUDE}/event/mouse_scroll.h
           ${INCLUDE}/event/direction.h
           ${INCLUDE}/event/button_action.h
           ${INCLUDE}/event/device_type.h
           ${INCLUDE}/event/modifier_value.h
           ${INCLUDE}/event/schema.h
           ${INCLUDE}/event/graph.h
           ${INCLUDE}/event_listener.h
           ${INCLUDE}/cli.h
           ${INCLUDE}/storage/json_storage.h
           ${INCLUDE}/storage/database_storage.h
           ${INCLUDE}/event/data.h
           ${INCLUDE}/event/entry.h
           ${INCLUDE}/storage/database_manager.h
           ${INCLUDE}/error.h
           ${INCLUDE}/async/container/locking_vector.h
           ${INCLUDE}/async/scheduler/interval.h
           ${INCLUDE}/async/scheduler/scheduler.h
)
set_property(TARGET ${LIBRARY_NAME} PROPERTY OUTPUT_NAME evget)

set(SCHEMA database/schema)
set(QUERIES database/queries)
set(SCHEMA_GENERATED schema)
set(QUERIES_GENERATED queries)
set(NAMESPACE evget::detail)

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

# Ensure that clang-tidy doesn't run on the generated files.
file(WRITE "${cmake_toolbelt_ret}/.clang-tidy" "Checks: '-*'")

toolbelt_add_dep(
    ${LIBRARY_NAME}
    spdlog
    VISIBILITY
    PUBLIC
    LINK_COMPONENTS
    spdlog::spdlog
    FIND_PACKAGE_ARGS
    REQUIRED
)
toolbelt_add_dep(
    ${LIBRARY_NAME}
    Boost
    VISIBILITY
    PUBLIC
    LINK_COMPONENTS
    Boost::headers
    FIND_PACKAGE_ARGS
    REQUIRED
)
toolbelt_add_dep(
    ${LIBRARY_NAME}
    nlohmann_json
    VISIBILITY
    PUBLIC
    LINK_COMPONENTS
    nlohmann_json::nlohmann_json
    FIND_PACKAGE_ARGS
    REQUIRED
)
toolbelt_add_dep(
    ${LIBRARY_NAME}
    SQLiteCpp
    VISIBILITY
    PUBLIC
    LINK_COMPONENTS
    SQLiteCpp
    FIND_PACKAGE_ARGS
    REQUIRED
)
toolbelt_add_dep(
    ${LIBRARY_NAME}
    OpenSSL
    VISIBILITY
    PUBLIC
    LINK_COMPONENTS
    openssl::openssl
    FIND_PACKAGE_ARGS
    REQUIRED
)
toolbelt_add_dep(
    ${LIBRARY_NAME}
    CLI11
    VISIBILITY
    PUBLIC
    LINK_COMPONENTS
    CLI11::CLI11
    FIND_PACKAGE_ARGS
    REQUIRED
)

file(STRINGS "../LICENSE" LICENSE)
list(GET LICENSE 0 LICENSE_NAME)
list(GET LICENSE 2 LICENSE_COPYRIGHT)
target_compile_definitions(${LIBRARY_NAME} PUBLIC EVGET_LICENSE="${LICENSE_NAME}")
target_compile_definitions(${LIBRARY_NAME} PUBLIC EVGET_COPYRIGHT="${LICENSE_COPYRIGHT}")
# x-release-please-start-version
target_compile_definitions(${LIBRARY_NAME} PUBLIC EVGET_VERSION="0.1.0")
# x-release-please-end

if(BUILD_TESTING)
    target_sources(
        ${TEST_EXECUTABLE_NAME}
        PUBLIC test/async/container/locking_vector.cpp
               test/async/scheduler/interval.cpp
               test/async/scheduler/scheduler.cpp
               test/database/sqlite/connection.cpp
               test/database/sqlite/migrate.cpp
               test/database/sqlite/query.cpp
               test/event/mouse_move.cpp
               test/event/mouse_click.cpp
               test/event/key.cpp
               test/event/mouse_scroll.cpp
               test/event/data.cpp
               test/common/database.h
               test/common/database.cpp
    )
    target_include_directories(${TEST_EXECUTABLE_NAME} PUBLIC test)
endif()

if(EVGET_INSTALL_LIB)
    install(TARGETS ${LIBRARY_NAME} FILE_SET headers)
endif()
