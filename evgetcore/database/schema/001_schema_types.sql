-- Enum type representing button action types.
create table button_action (
    id integer primary key,
    enum text not null unique
);
insert into button_action values (1, 'Press'),
    (2, 'Release'),
    (3, 'Repeat');

-- Enum type representing device types.
create table device_type (
    id integer primary key,
    enum text not null unique
);
insert into device_type values (1, 'Mouse'),
    (2, 'Keyboard'),
    (3, 'Touchpad'),
    (4, 'Touchscreen');
