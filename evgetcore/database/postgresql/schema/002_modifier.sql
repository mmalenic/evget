-- A modifier table containing the type of modifier in effect.
create table evget.modifier (
    modifier_id uuid primary key default gen_random_uuid(),
    modifier_value evget.modifier_type not null
);