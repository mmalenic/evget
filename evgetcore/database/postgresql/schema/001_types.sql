create schema evget;

-- Enum type representing possible modifier values.
create type evget.modifier_type as enum (
    'Shift',
    -- X11 - Lock
    'CapsLock',
    'Control',
    -- X11 - Mod1, Mac - Option
    'Alt',
    -- X11 - Mod2
    'NumLock',
    'Mod3',
    -- X11 - Mod4, Mac - Command, Windows - Windows key
    'Super',
    'Mod5'
);

-- Enum type representing button action types.
create type evget.button_action as enum (
    'Press',
    'Release',
    'Repeat'
);

-- Enum type representing device types.
create type evget.device_type as enum (
    'Mouse',
    'Keyboard',
    'Touchpad',
    'Touchscreen'
);

-- Enum type representing scroll directions.
create type evget.direction as enum (
    'Down',
    'Left',
    'Right',
    'Up'
);

