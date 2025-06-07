-- A key table for key events.
create table key (
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
    device_type integer not null references device_type(id),
    button_id integer,
    button_name text,
    button_action integer not null references button_action(id),
    character text
);

-- Linking table to modifier
create table key_modifier (
    uuid text primary key,
    key_uuid text not null references key(uuid),
    modifier_id text not null references modifier(id)
);
