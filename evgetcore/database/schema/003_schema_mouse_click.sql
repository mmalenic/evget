-- A table for mouse click events.
create table mouse_click (
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
    info text,
    device_type integer not null references device_type(id),
    button_id integer,
    button_name text,
    button_action integer not null references button_action(id)
);

-- Linking table to modifier
create table mouse_click_modifier (
    uuid text primary key,
    mouse_click_uuid integer not null references mouse_click(uuid),
    modifier_id integer not null references modifier(id)
);
