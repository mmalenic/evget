-- A modifier table containing the type of modifier in effect.
create table modifier (
    id integer primary key,
    value text not null unique
);
insert into modifier values (1, 'Shift'),
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
