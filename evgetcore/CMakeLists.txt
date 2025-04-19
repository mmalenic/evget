set(INCLUDE include/evgetcore)
set(SRC src)

target_sources(
    ${PROJECT_NAME}
        PUBLIC
        ${INCLUDE}/database/sqlite/Connection.h
        ${INCLUDE}/database/sqlite/Query.h
        ${INCLUDE}/database/Connection.h
        ${INCLUDE}/database/Error.h
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
    ${INCLUDE}/UnsupportedOperationException.h
        ${INCLUDE}/Parser.h
    ${INCLUDE}/Event/ModifierValue.h
    ${INCLUDE}/Event/Schema.h
    ${INCLUDE}/Event/Graph.h
        ${INCLUDE}/Storage/JsonStorage.h
        ${INCLUDE}/Storage/DatabaseStorage.h
        ${INCLUDE}/Storage/Error.h
        ${INCLUDE}/Event/Data.h
        ${INCLUDE}/Event/Entry.h
        ${INCLUDE}/Storage/DatabaseManager.h
        ${INCLUDE}/Error.h
        ${INCLUDE}/async/container/LockingVector.h
        ${INCLUDE}/async/scheduler/Interval.h
        ${INCLUDE}/async/scheduler/Scheduler.h
)
target_include_directories(${PROJECT_NAME} PUBLIC include)

target_sources(
        ${PROJECT_NAME}
        PRIVATE
        ${SRC}/Parser.cpp
        ${SRC}/Event/Key.cpp
        ${SRC}/Event/MouseClick.cpp
        ${SRC}/Event/MouseMove.cpp
        ${SRC}/Event/MouseScroll.cpp
        ${SRC}/Event/Data.cpp
        ${SRC}/UnsupportedOperationException.cpp
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

set(SCHEMA database/schema)
set(QUERIES database/queries)
set(SCHEMA_GENERATED schema)
set(QUERIES_GENERATED queries)
set(NAMESPACE EvgetCore::Database::detail)

toolbelt_embed(
        ${SCHEMA_GENERATED}/initialize.h initialize
        EMBED ${SCHEMA}/000_schema_types.sql
        ${SCHEMA}/001_schema_modifier.sql
        ${SCHEMA}/002_schema_key.sql
        ${SCHEMA}/003_schema_mouse_click.sql
        ${SCHEMA}/004_schema_mouse_move.sql
        ${SCHEMA}/005_schema_mouse_scroll.sql
        NAMESPACE ${NAMESPACE}
        TARGET ${PROJECT_NAME}
)
toolbelt_embed(${QUERIES_GENERATED}/insert_key.h insert_key NAMESPACE ${NAMESPACE} EMBED ${QUERIES}/insert_key.sql TARGET ${PROJECT_NAME})
toolbelt_embed(${QUERIES_GENERATED}/insert_key_modifier.h insert_key_modifier NAMESPACE ${NAMESPACE} EMBED ${QUERIES}/insert_key_modifier.sql  TARGET ${PROJECT_NAME})
toolbelt_embed(${QUERIES_GENERATED}/insert_mouse_click.h insert_mouse_click NAMESPACE ${NAMESPACE} EMBED ${QUERIES}/insert_mouse_click.sql  TARGET ${PROJECT_NAME})
toolbelt_embed(${QUERIES_GENERATED}/insert_mouse_click_modifier.h insert_mouse_click_modifier NAMESPACE ${NAMESPACE} EMBED ${QUERIES}/insert_mouse_click_modifier.sql  TARGET ${PROJECT_NAME})
toolbelt_embed(${QUERIES_GENERATED}/insert_mouse_move.h insert_mouse_move NAMESPACE ${NAMESPACE} EMBED ${QUERIES}/insert_mouse_move.sql  TARGET ${PROJECT_NAME})
toolbelt_embed(${QUERIES_GENERATED}/insert_mouse_move_modifier.h insert_mouse_move_modifier NAMESPACE ${NAMESPACE} EMBED ${QUERIES}/insert_mouse_move_modifier.sql  TARGET ${PROJECT_NAME})
toolbelt_embed(${QUERIES_GENERATED}/insert_mouse_scroll.h insert_mouse_scroll NAMESPACE ${NAMESPACE} EMBED ${QUERIES}/insert_mouse_scroll.sql  TARGET ${PROJECT_NAME})
toolbelt_embed(${QUERIES_GENERATED}/insert_mouse_scroll_modifier.h insert_mouse_scroll_modifier NAMESPACE ${NAMESPACE} EMBED ${QUERIES}/insert_mouse_scroll_modifier.sql  TARGET ${PROJECT_NAME})
target_include_directories(${PROJECT_NAME} PRIVATE ${cmake_toolbelt_ret})

toolbelt_add_dep(${PROJECT_NAME} spdlog VISIBILITY PUBLIC LINK_COMPONENTS spdlog::spdlog)
toolbelt_add_dep(
    ${PROJECT_NAME}
    Boost
        VISIBILITY
        PUBLIC
        LINK_COMPONENTS
        Boost::headers
)
toolbelt_add_dep(${PROJECT_NAME} date VISIBILITY PUBLIC LINK_COMPONENTS date::date)
toolbelt_add_dep(${PROJECT_NAME} nlohmann_json VISIBILITY PUBLIC LINK_COMPONENTS nlohmann_json::nlohmann_json)
toolbelt_add_dep(${PROJECT_NAME} SQLiteCpp VISIBILITY PUBLIC LINK_COMPONENTS SQLiteCpp)

#if(BUILD_TESTING)
#    set(TEST_EXECUTABLE_NAME evgetcoretest)
#
#    add_executable(
#            ${TEST_EXECUTABLE_NAME}
#            Event/MouseMoveTest.cpp
#            Event/MouseClickTest.cpp
#            Event/KeyTest.cpp
#            Event/MouseScrollTest.cpp
#            Event/DataTest.cpp
#            Event/FieldTest.cpp
#            Event/RelationTest.cpp
#            utils/EventTestUtils.h
#            utils/EventTestUtils.cpp
#    )
#    target_include_directories(${TEST_EXECUTABLE_NAME} PUBLIC utils)
#
#    toolbelt_setup_gtest(${TEST_EXECUTABLE_NAME})
#endif()
