-- A table for mouse scroll events.
create table mouse_scroll (
    id integer primary key,
    interval real,
    timestamp text not null,
    device_type integer not null references device_type(id),
    position_x real,
    position_y real,
    scroll_down real,
    scroll_left real,
    scroll_right real,
    scroll_up real,
    device_name text,
    focus_window_name text,
    focus_window_position_x real,
    focus_window_position_y real,
    focus_window_width real,
    info text
);

-- Linking table to modifier
create table mouse_scroll_modifier (
    id integer primary key,
    mouse_click_id integer not null references mouse_scroll(id),
    modifier_id integer not null references modifier(id)
);