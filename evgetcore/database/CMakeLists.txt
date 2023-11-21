set(SCHEMA generated/schema)
set(QUERIES generated/queries)
set(NAMESPACE EvgetCore::Database::detail)

create_header_file(schema/001_schema_types.sql ${SCHEMA}/schema_types.h schema_types ${NAMESPACE})
create_header_file(schema/002_schema_modifier.sql ${SCHEMA}/schema_modifier.h schema_modifier ${NAMESPACE})
create_header_file(schema/003_schema_key.sql ${SCHEMA}/schema_key.h schema_key ${NAMESPACE})
create_header_file(schema/004_schema_mouse_click.sql ${SCHEMA}/schema_mouse_click.h schema_mouse_click ${NAMESPACE})
create_header_file(schema/005_schema_mouse_move.sql ${SCHEMA}/schema_mouse_move.h schema_mouse_move ${NAMESPACE})
create_header_file(schema/006_schema_mouse_scroll.sql ${SCHEMA}/schema_mouse_scroll.h schema_mouse_scroll ${NAMESPACE})

create_header_file(queries/insert_key.sql ${QUERIES}/insert_key.h insert_key ${NAMESPACE})
create_header_file(queries/insert_key_modifier.sql ${QUERIES}/insert_key_modifier.h insert_key_modifier ${NAMESPACE})
create_header_file(queries/insert_mouse_click.sql ${QUERIES}/insert_mouse_click.h insert_mouse_click ${NAMESPACE})
create_header_file(queries/insert_mouse_click_modifier.sql ${QUERIES}/insert_mouse_click_modifier.h insert_mouse_click_modifier ${NAMESPACE})
create_header_file(queries/insert_mouse_move.sql ${QUERIES}/insert_mouse_move.h insert_mouse_move ${NAMESPACE})
create_header_file(queries/insert_mouse_move_modifier.sql ${QUERIES}/insert_mouse_move_modifier.h insert_mouse_move_modifier ${NAMESPACE})
create_header_file(queries/insert_mouse_scroll.sql ${QUERIES}/insert_mouse_scroll.h insert_mouse_scroll ${NAMESPACE})
create_header_file(queries/insert_mouse_scroll_modifier.sql ${QUERIES}/insert_mouse_scroll_modifier.h insert_mouse_scroll_modifier ${NAMESPACE})

target_sources(
        ${PROJECT_NAME} PRIVATE
        ${SCHEMA}/schema_types.h
        ${SCHEMA}/schema_modifier.h
        ${SCHEMA}/schema_key.h
        ${SCHEMA}/schema_mouse_click.h
        ${SCHEMA}/schema_mouse_move.h
        ${SCHEMA}/schema_mouse_scroll.h
        ${QUERIES}/insert_key.h
        ${QUERIES}/insert_key_modifier.h
        ${QUERIES}/insert_mouse_click.h
        ${QUERIES}/insert_mouse_click_modifier.h
        ${QUERIES}/insert_mouse_move.h
        ${QUERIES}/insert_mouse_move_modifier.h
        ${QUERIES}/insert_mouse_scroll.h
        ${QUERIES}/insert_mouse_scroll_modifier.h
)
target_include_directories(${PROJECT_NAME} PUBLIC generated)