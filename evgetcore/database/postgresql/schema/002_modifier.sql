-- A modifier table containing the type of modifier in effect.
create table if not exists modifier (
    modifier_id uuid primary key default gen_random_uuid(),
    modifier_value modifier_type not null
);