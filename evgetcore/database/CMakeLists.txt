set(SCHEMA generated/schema)
set(QUERIES generated/queries)

create_header_file(queries/insert_key.sql ${QUERIES}/insert_key.h insert_key EvgetCore::Database::detail)
create_header_file(queries/insert_key_modifier.sql ${QUERIES}/insert_key_modifier.h insert_key_modifier EvgetCore::Database::detail)
create_header_file(queries/insert_mouse_click.sql ${QUERIES}/insert_mouse_click.h insert_mouse_click EvgetCore::Database::detail)
create_header_file(queries/insert_mouse_click_modifier.sql ${QUERIES}/insert_mouse_click_modifier.h insert_mouse_click_modifier EvgetCore::Database::detail)
create_header_file(queries/insert_mouse_move.sql ${QUERIES}/insert_mouse_move.h insert_mouse_move EvgetCore::Database::detail)
create_header_file(queries/insert_mouse_move_modifier.sql ${QUERIES}/insert_mouse_move_modifier.h insert_mouse_move_modifier EvgetCore::Database::detail)
create_header_file(queries/insert_mouse_scroll.sql ${QUERIES}/insert_mouse_scroll.h insert_mouse_scroll EvgetCore::Database::detail)
create_header_file(queries/insert_mouse_scroll_modifier.sql ${QUERIES}/insert_mouse_scroll_modifier.h insert_mouse_scroll_modifier EvgetCore::Database::detail)

target_sources(
        ${PROJECT_NAME} PRIVATE
        ${QUERIES}/insert_key.h
        ${QUERIES}/insert_key_modifier.h
        ${QUERIES}/insert_mouse_click.h
        ${QUERIES}/insert_mouse_click_modifier.h
        ${QUERIES}/insert_mouse_move.h
        ${QUERIES}/insert_mouse_move_modifier.h
        ${QUERIES}/insert_mouse_scroll.h
        ${QUERIES}/insert_mouse_scroll_modifier.h
)
target_include_directories(${PROJECT_NAME} PRIVATE generated)