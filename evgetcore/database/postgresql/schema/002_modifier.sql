-- A modifier table containing the type of modifier in effect.
create table evget.modifier (
    id integer primary,
    value evget.modifier_type not null
);