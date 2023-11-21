set(SCHEMA generated/schema)
set(QUERIES generated/queries)
set(NAMESPACE EvgetCore::Database::detail)

create_header_file(
        ${SCHEMA}/initialize.h initialize ${NAMESPACE}
        TARGET_FILE_NAMES schema/001_schema_types.sql
        schema/002_schema_modifier.sql
        schema/003_schema_key.sql
        schema/004_schema_mouse_click.sql
        schema/005_schema_mouse_move.sql
        schema/006_schema_mouse_scroll.sql
)

create_header_file(${QUERIES}/insert_key.h insert_key ${NAMESPACE} TARGET_FILE_NAMES queries/insert_key.sql)
create_header_file(${QUERIES}/insert_key_modifier.h insert_key_modifier ${NAMESPACE} TARGET_FILE_NAMES queries/insert_key_modifier.sql)
create_header_file(${QUERIES}/insert_mouse_click.h insert_mouse_click ${NAMESPACE} TARGET_FILE_NAMES queries/insert_mouse_click.sql)
create_header_file(${QUERIES}/insert_mouse_click_modifier.h insert_mouse_click_modifier ${NAMESPACE} TARGET_FILE_NAMES queries/insert_mouse_click_modifier.sql)
create_header_file(${QUERIES}/insert_mouse_move.h insert_mouse_move ${NAMESPACE} TARGET_FILE_NAMES queries/insert_mouse_move.sql)
create_header_file(${QUERIES}/insert_mouse_move_modifier.h insert_mouse_move_modifier ${NAMESPACE} TARGET_FILE_NAMES queries/insert_mouse_move_modifier.sql)
create_header_file(${QUERIES}/insert_mouse_scroll.h insert_mouse_scroll ${NAMESPACE} TARGET_FILE_NAMES queries/insert_mouse_scroll.sql)
create_header_file(${QUERIES}/insert_mouse_scroll_modifier.h insert_mouse_scroll_modifier ${NAMESPACE} TARGET_FILE_NAMES queries/insert_mouse_scroll_modifier.sql)

target_sources(
        ${PROJECT_NAME} PRIVATE
        ${SCHEMA}/initialize.h
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