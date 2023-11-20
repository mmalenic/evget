-- Enum table representing possible modifier values.
create table modifier_type (
    id integer primary key,
    enum text not null unique
);
insert into modifier_type values (1, 'Shift'),
    -- X11 - Lock
    (2, 'CapsLock'),
    (3, 'Control'),
    -- X11 - Mod1, Mac - Option
    (4, 'Alt'),
    -- X11 - Mod2
    (5, 'NumLock'),
    (6, 'Mod3'),
    -- X11 - Mod4, Mac - Command, Windows - Windows key
    (7, 'Super'),
    (8, 'Mod5');

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

-- Enum type representing scroll directions.
create table direction (
    id integer primary key,
    enum text not null unique
);
insert into direction values (1, 'Down'),
    (2, 'Left'),
    (3, 'Right'),
    (4, 'Up');

