-- A modifier table containing the type of modifier in effect.
create table modifier (
    id integer primary key,
    value text not null unique
);
insert into modifier values (0, 'Shift'),
    -- X11 - Lock
    (1, 'CapsLock'),
    (2, 'Control'),
    -- X11 - Mod1, Mac - Option
    (3, 'Alt'),
    -- X11 - Mod2
    (4, 'NumLock'),
    (5, 'Mod3'),
    -- X11 - Mod4, Mac - Command, Windows - Windows key
    (6, 'Super'),
    (7, 'Mod5');
