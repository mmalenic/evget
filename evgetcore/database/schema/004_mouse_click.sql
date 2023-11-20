-- A table for mouse click events.
create table mouse_click (
    id integer primary key,
    interval real,
    timestamp text not null,
    device_type integer not null references device_type(id),
    position_x real,
    position_y real,
    button_action integer not null references button_action(id),
    button_id integer,
    button_name text,
    device_name text,
    focus_window_name text,
    focus_window_position_x real,
    focus_window_position_y real,
    focus_window_width real,
    focus_window_height real,
    info text
);

-- Linking table to modifier
create table mouse_click_modifier (
    id integer primary key,
    mouse_click_id integer not null references mouse_click(id),
    modifier_id integer not null references modifier(id)
);
