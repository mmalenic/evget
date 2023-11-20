create_header_file(queries/insert_key.sql generated/insert_key.h insert_key EvgetCore::Database::detail)
create_header_file(queries/insert_key_modifier.sql generated/insert_key_modifier.h insert_key_modifier EvgetCore::Database::detail)
create_header_file(queries/insert_mouse_click.sql generated/insert_mouse_click.h insert_mouse_click EvgetCore::Database::detail)
create_header_file(queries/insert_mouse_click_modifier.sql generated/insert_mouse_click_modifier.h insert_mouse_click_modifier EvgetCore::Database::detail)
create_header_file(queries/insert_mouse_move.sql generated/insert_mouse_move.h insert_mouse_move EvgetCore::Database::detail)
create_header_file(queries/insert_mouse_move_modifier.sql generated/insert_mouse_move_modifier.h insert_mouse_move_modifier EvgetCore::Database::detail)
create_header_file(queries/insert_mouse_scroll.sql generated/insert_mouse_scroll.h insert_mouse_scroll EvgetCore::Database::detail)
create_header_file(queries/insert_mouse_scroll_modifier.sql generated/insert_mouse_scroll_modifier.h insert_mouse_scroll_modifier EvgetCore::Database::detail)

target_sources(
        ${PROJECT_NAME} PRIVATE
        generated/insert_key.h
        generated/insert_key_modifier.h
        generated/insert_mouse_click.h
        generated/insert_mouse_click_modifier.h
        generated/insert_mouse_move.h
        generated/insert_mouse_move_modifier.h
        generated/insert_mouse_scroll.h
        generated/insert_mouse_scroll_modifier.h
)