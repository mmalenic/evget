-- A table for mouse move events.
create table mouse_move (
    id integer primary key,
    interval real,
    timestamp text not null,
    device_type integer not null references device_type(id),
    position_x real,
    position_y real,
    device_name text,
    focus_window_name text,
    focus_window_position_x real,
    focus_window_position_y real,
    focus_window_width real,
    info text
);

-- Linking table to modifier
create table mouse_move_modifier (
    id integer primary key,
    mouse_click_id integer not null references mouse_move(id),
    modifier_id integer not null references modifier(id)
);