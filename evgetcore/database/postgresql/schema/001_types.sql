do $$ begin
    -- Enum type representing possible modifier values.
    create type modifier_type as enum (
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
exception
    when duplicate_object then null;
end $$;