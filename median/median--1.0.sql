
CREATE OR REPLACE FUNCTION median_transfn_int4(state internal, val int4)
RETURNS internal
AS 'MODULE_PATHNAME', 'median_transfn_int4'
LANGUAGE C IMMUTABLE;

CREATE OR REPLACE FUNCTION median_finalfn_int4(state internal, val int4)
RETURNS int4
AS 'MODULE_PATHNAME', 'median_finalfn_int4'
LANGUAGE C IMMUTABLE;

DROP AGGREGATE IF EXISTS median (int4);
CREATE AGGREGATE median (int4)
(
    sfunc = median_transfn_int4,
    stype = internal,
    finalfunc = median_finalfn_int4,
    finalfunc_extra
);


CREATE OR REPLACE FUNCTION median_transfn_float8(state internal, val float8)
RETURNS internal
AS 'MODULE_PATHNAME', 'median_transfn_float8'
LANGUAGE C IMMUTABLE;

CREATE OR REPLACE FUNCTION median_finalfn_float8(state internal, val float8)
RETURNS float8
AS 'MODULE_PATHNAME', 'median_finalfn_float8'
LANGUAGE C IMMUTABLE;

DROP AGGREGATE IF EXISTS median (float8);
CREATE AGGREGATE median (float8)
(
    sfunc = median_transfn_float8,
    stype = internal,
    finalfunc = median_finalfn_float8,
    finalfunc_extra
);


CREATE OR REPLACE FUNCTION median_transfn_text(state internal, val text)
RETURNS internal
AS 'MODULE_PATHNAME', 'median_transfn_text'
LANGUAGE C IMMUTABLE;

CREATE OR REPLACE FUNCTION median_finalfn_text(state internal, val text)
RETURNS text
AS 'MODULE_PATHNAME', 'median_finalfn_text'
LANGUAGE C IMMUTABLE;

DROP AGGREGATE IF EXISTS median (text);
CREATE AGGREGATE median (text)
(
    sfunc = median_transfn_text,
    stype = internal,
    finalfunc = median_finalfn_text,
    finalfunc_extra
);


CREATE OR REPLACE FUNCTION median_transfn_timestamp(state internal, val TIMESTAMPTZ)
RETURNS internal
AS 'MODULE_PATHNAME', 'median_transfn_timestamp'
LANGUAGE C IMMUTABLE;

CREATE OR REPLACE FUNCTION median_finalfn_timestamp(state internal, val TIMESTAMPTZ)
RETURNS TIMESTAMPTZ	
AS 'MODULE_PATHNAME', 'median_finalfn_timestamp'
LANGUAGE C IMMUTABLE;

DROP AGGREGATE IF EXISTS median (TIMESTAMPTZ);
CREATE AGGREGATE median (TIMESTAMPTZ)
(
    sfunc = median_transfn_timestamp,
    stype = internal,
    finalfunc = median_finalfn_timestamp,
    finalfunc_extra
);