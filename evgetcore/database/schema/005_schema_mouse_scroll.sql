-- A table for mouse scroll events.
create table mouse_scroll (
    uuid text primary key,
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
    focus_window_height real,
    info text,
    scroll_vertical real,
    scroll_horizontal real
);

-- Linking table to modifier
create table mouse_scroll_modifier (
    uuid text primary key,
    mouse_click_id integer not null references mouse_scroll(id),
    modifier_id integer not null references modifier(id)
);
