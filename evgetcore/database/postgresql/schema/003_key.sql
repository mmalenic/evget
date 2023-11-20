-- A key table for key events.
create table evget.key (
    id uuid primary key default gen_random_uuid(),
    interval interval,
    timestamp timestamptz not null,
    device_type evget.device_type not null,
    position_x double precision,
    position_y double precision,
    button_action evget.button_action not null,
    button_id int,
    button_name text,
    character text,
    device_name text,
    focus_window_name text,
    focus_window_position_x double precision,
    focus_window_position_y double precision,
    focus_window_width double precision,
    info json
);