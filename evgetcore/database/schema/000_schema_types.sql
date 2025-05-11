-- Enum type representing button action types.
create table button_action (
    id integer primary key,
    enum text not null unique
);
insert into button_action values (0, 'Press'),
    (1, 'Release'),
    (2, 'Repeat');

-- Enum type representing device types.
create table device_type (
    id integer primary key,
    enum text not null unique
);
insert into device_type values (0, 'Mouse'),
    (1, 'Keyboard'),
    (2, 'Touchpad'),
    (3, 'Touchscreen')
    (4, 'Unknown');
