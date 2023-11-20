-- A modifier table containing the type of modifier in effect.
create table modifier (
    id integer primary key,
    value integer not null references modifier_type(id)
);