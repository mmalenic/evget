-- A table for mouse move events.
create table mouse_move (
    uuid text primary key,
    interval real,
    timestamp text not null,
    position_x real,
    position_y real,
    device_name text,
    focus_window_name text,
    focus_window_position_x real,
    focus_window_position_y real,
    focus_window_width real,
    focus_window_height real,
    screen real,
    device_type integer not null references device_type(id)
);

-- Linking table to modifier
create table mouse_move_modifier (
    uuid text primary key,
    mouse_click_uuid integer not null references mouse_move(uuid),
    modifier_id integer not null references modifier(id)
);
