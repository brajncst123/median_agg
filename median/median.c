#include <postgres.h>
#include <fmgr.h>
#include "utils/array.h"
#include "utils/lsyscache.h"
#include "port.h"
#include "nodes/execnodes.h"
#include "utils/builtins.h" 
#include "utils/timestamp.h"


#if PG_VERSION_NUM < 120000 || PG_VERSION_NUM >= 150000
#error "Unsupported PostgreSQL version. Use version 12."
#endif

#ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC;
#endif

#define	MIN_DATA	1000

/*
 *  ArrayState structure  
 * To store the data in extendable array at run time
*/
typedef struct mdstate
{
	void* data;      /* column data array */
	int	maxelem;	/* size of the data array */
	int	nelem;		/* number of data */
} mdstate;

typedef struct mdstatetext
{
	char** data;    /* column data array */
	int	maxelem;	/* size of the data array */
	int	nelem;		/* number of data */
} mdstatetext;

/* TEXT comparator utility for quick sort algorithm */
#define timestamptz_cmp_internal(dt1, dt2)	timestamp_cmp_internal(dt1, dt2);

/* Int comparator utility for quick sort algorithm */
static int32
int32_comparator(const void* a, const void* b)
{
	int32 af = (*(int32*)a);
	int32 bf = (*(int32*)b);
	return (af > bf) - (af < bf);
}

/* double comparator utility for quick sort algorithm */
static float8
float8_comparator(const void* a, const void* b)
{
	float8 af = (*(float8*)a);
	float8 bf = (*(float8*)b);
	return (float8)(af > bf) - (af < bf);
}

PG_FUNCTION_INFO_V1(median_transfn_int4);

/*
 * Median Aggregate state transfer function.
 *
 * This trans function is called for every tuple values.
 * On the very first call the aggregate state gets initilized,
 * Afterwords all the subsequent calls append each tuple values
 * in state array.
 */

Datum
median_transfn_int4(PG_FUNCTION_ARGS)
{
	MemoryContext agg_context;
	mdstate* state;
	MemoryContext	oldcontext;
	int32* data;

	if (!AggCheckCallContext(fcinfo, &agg_context))
		elog(ERROR, "median_transfn called in non-aggregate context");
	
	/* skip NULL values */
	if (PG_ARGISNULL(1))
	{
		if (PG_ARGISNULL(0))
			PG_RETURN_NULL();
		else
			/* in case of NULL value, just return preserved state */
			PG_RETURN_DATUM(PG_GETARG_DATUM(0));
	}

	oldcontext = MemoryContextSwitchTo(agg_context);

	if (PG_ARGISNULL(0))
	{
		state = (mdstate*)palloc(sizeof(mdstate));
		state->data = palloc(MIN_DATA * sizeof(int32));
		state->maxelem = MIN_DATA;
		state->nelem = 0;
	}
	else
		state = (mdstate*)PG_GETARG_POINTER(0);

	if (state->nelem == state->maxelem)
	{
		state->maxelem *= 2;
		state->data = repalloc(state->data,
			sizeof(int32) * state->maxelem);
	}

	Assert(state->nelem < state->maxelem);

	data = (int32*)state->data;
	data[state->nelem++] = PG_GETARG_INT32(1);

	MemoryContextSwitchTo(oldcontext);

	PG_RETURN_POINTER(state);
}

PG_FUNCTION_INFO_V1(median_finalfn_int4);

/*
 * Median final function.
 * This final function is called after all the 
 * values in the median set has been collected by the 
 * state transfer function.
 */
Datum
median_finalfn_int4(PG_FUNCTION_ARGS)
{
	MemoryContext agg_context;
	int	idx = 0;
	mdstate* state;
	int32* data;

	if (!AggCheckCallContext(fcinfo, &agg_context))
		elog(ERROR, "median_finalfn called in non-aggregate context");

	if (PG_ARGISNULL(0))
		PG_RETURN_NULL();

	state = (mdstate*)PG_GETARG_POINTER(0);
	data = (int32*)state->data;

	qsort(state->data, state->nelem, sizeof(int32), &int32_comparator);

	int mid = (state->nelem) / 2;

	if (state->nelem % 2 == 1) {
		int32 v1 = data[mid];
		PG_RETURN_INT32(v1);
	}
	else {
		int32 v1 = ((int64)((int64)data[mid] + (int64)data[mid - 1])) / 2;
		PG_RETURN_INT32(v1);
	}
}

PG_FUNCTION_INFO_V1(median_transfn_float8);

/*
 * Median Aggregate state transfer function for float.
 *
 * This trans function is called for every tuple values.
 * On the very first call the aggregate state gets initilized,
 * Afterwords all the subsequent calls append each tuple values
 * in state array.
 */
Datum
median_transfn_float8(PG_FUNCTION_ARGS)
{
	MemoryContext agg_context;
	mdstate* state;
	MemoryContext	oldcontext;
	float8* data;

	if (!AggCheckCallContext(fcinfo, &agg_context))
		elog(ERROR, "median_transfn called in non-aggregate context");

	/* skip NULL values */
	if (PG_ARGISNULL(1))
	{
		if (PG_ARGISNULL(0))
			PG_RETURN_NULL();
		else
			/* in case of NULL value, just return preserved state */
			PG_RETURN_DATUM(PG_GETARG_DATUM(0));
	}

	oldcontext = MemoryContextSwitchTo(agg_context);

	if (PG_ARGISNULL(0))
	{
		state = (mdstate*)palloc(sizeof(mdstate));

		state->data = palloc(MIN_DATA * sizeof(float8));
		state->maxelem = MIN_DATA;
		state->nelem = 0;
	}
	else
		state = (mdstate*)PG_GETARG_POINTER(0);

	if (state->nelem == state->maxelem)
	{
		state->maxelem *= 2;
		state->data = repalloc(state->data,
			sizeof(float8) * state->maxelem);
	}

	Assert(state->nelem < state->maxelem);

	data = (float8*)state->data;
	data[state->nelem++] = PG_GETARG_FLOAT8(1);

	MemoryContextSwitchTo(oldcontext);

	PG_RETURN_POINTER(state);
}

PG_FUNCTION_INFO_V1(median_finalfn_float8);

/*
 * Median Aggregate final function for float.
 * This final function is called after all the 
 * values in the median set has been collected by the 
 * state transfer function.
 */
Datum
median_finalfn_float8(PG_FUNCTION_ARGS)
{
	MemoryContext agg_context;
	int				idx = 0;
	mdstate* state;
	float8* data;

	if (!AggCheckCallContext(fcinfo, &agg_context))
		elog(ERROR, "median_finalfn called in non-aggregate context");

	if (PG_ARGISNULL(0))
		PG_RETURN_NULL();

	state = (mdstate*)PG_GETARG_POINTER(0);
	data = (float8*)state->data;

	qsort(state->data, state->nelem, sizeof(float8), &float8_comparator);

	int mid = (state->nelem) / 2;

	if (state->nelem % 2 == 1) {
		float8 v1 = data[mid];
		PG_RETURN_FLOAT8(v1);
	}
	else {
		float8 v1 = (data[mid] + data[mid - 1]) / 2.0;
		PG_RETURN_FLOAT8(v1);
	}
}

PG_FUNCTION_INFO_V1(median_transfn_text);

/*
 * Median Aggregate state transfer function for TEXT.
 *
 * This trans function is called for every tuple values.
 * On the very first call the aggregate state gets initilized,
 * Afterwords all the subsequent calls append each tuple values
 * in state array.
 */

Datum
median_transfn_text(PG_FUNCTION_ARGS)
{
	MemoryContext agg_context;
	mdstatetext* state;
	MemoryContext	oldcontext;
	char** data;
	char* string = text_to_cstring(PG_GETARG_TEXT_PP(1));
	int string_len = strlen(string);

	if (!AggCheckCallContext(fcinfo, &agg_context))
		elog(ERROR, "median_transfn called in non-aggregate context");

	/* skip NULL values */
	if (PG_ARGISNULL(1))
	{
		if (PG_ARGISNULL(0))
			PG_RETURN_NULL();
		else
			/* in case of NULL value, just return preserved state */
			PG_RETURN_DATUM(PG_GETARG_DATUM(0));
	}

	oldcontext = MemoryContextSwitchTo(agg_context);

	if (PG_ARGISNULL(0))
	{
		state = (mdstatetext*)palloc(sizeof(mdstatetext));

		state->data = (char **)palloc(MIN_DATA * sizeof(char *));
		state->maxelem = MIN_DATA;
		state->nelem = 0;
	}
	else
		state = (mdstatetext*)PG_GETARG_POINTER(0);

	/* Extend the state array allocation */
	if (state->nelem == state->maxelem)
	{
		state->maxelem *= 2;
		state->data = repalloc(state->data,
			 sizeof(char *) * state->maxelem);
	}

	Assert(state->nelem < state->maxelem);

	/* make sure to cast the array to (int32 *) before updating it */
	data = state->data;
	data[state->nelem] = (char*)palloc(sizeof(char) * string_len);
	strcpy(data[state->nelem],string);
	state->nelem++;

	MemoryContextSwitchTo(oldcontext);

	PG_RETURN_POINTER(state);
}

PG_FUNCTION_INFO_V1(median_finalfn_text);

/*
 * Median final function for TEXT.
 * This final function is called after all the 
 * values in the median set has been collected by the 
 * state transfer function.
 */
Datum
median_finalfn_text(PG_FUNCTION_ARGS)
{
	MemoryContext agg_context;
	int				idx = 0;
	mdstatetext* state;
	char** data;

	if (!AggCheckCallContext(fcinfo, &agg_context))
		elog(ERROR, "median_finalfn called in non-aggregate context");

	if (PG_ARGISNULL(0))
		PG_RETURN_NULL();

	state = (mdstatetext*)PG_GETARG_POINTER(0);
	data = state->data;

	qsort(data, state->nelem, sizeof(char *), &pg_qsort_strcmp);

	int mid = (state->nelem) / 2;

	if (state->nelem % 2 == 1) {
		char *v1 = data[mid];
		text *textstring = cstring_to_text(v1);
		PG_RETURN_TEXT_P(textstring);
	}
	else {  /* TODO: add the logic of even number of TEXT rows */
		char* v1 = data[mid];
		text* textstring = cstring_to_text(v1);
		PG_RETURN_TEXT_P(textstring);
	}
}

PG_FUNCTION_INFO_V1(median_transfn_timestamp);

/*
 * Median Aggregate state transfer function for Timestamp.
 *
 * This trans function is called for every tuple values.
 * On the very first call the aggregate state gets initilized,
 * Afterwords all the subsequent calls append each tuple values
 * in state array.
 */

Datum
median_transfn_timestamp(PG_FUNCTION_ARGS)
{
	MemoryContext agg_context;
	mdstate* state;
	MemoryContext	oldcontext;
	TimestampTz* data;

	if (!AggCheckCallContext(fcinfo, &agg_context))
		elog(ERROR, "median_transfn called in non-aggregate context");

	/* OK, we do want to skip NULL values altogether */
	if (PG_ARGISNULL(1))
	{
		if (PG_ARGISNULL(0))
			PG_RETURN_NULL();
		else
			/* in case of NULL value, just return preserved state */
			PG_RETURN_DATUM(PG_GETARG_DATUM(0));
	}

	oldcontext = MemoryContextSwitchTo(agg_context);

	if (PG_ARGISNULL(0))
	{
		state = (mdstate*)palloc(sizeof(mdstate));
		state->data = palloc(MIN_DATA * sizeof(TimestampTz));
		state->maxelem = MIN_DATA;
		state->nelem = 0;
	}
	else
		state = (mdstate*)PG_GETARG_POINTER(0);

	/* we can be sure the value is not null (see the check above) */
	if (state->nelem == state->maxelem)
	{
		state->maxelem *= 2;
		state->data = repalloc(state->data,
			sizeof(TimestampTz) * state->maxelem);
	}

	Assert(state->nelem < state->maxelem);

	data = (TimestampTz*)state->data;
	data[state->nelem++] = PG_GETARG_TIMESTAMPTZ(1);

	MemoryContextSwitchTo(oldcontext);

	PG_RETURN_POINTER(state);
}

PG_FUNCTION_INFO_V1(median_finalfn_timestamp);

/*
 * Median final function for Timestamp.
 * This final function is called after all the 
 * values in the median set has been collected by the 
 * state transfer function.
 */

Datum
median_finalfn_timestamp(PG_FUNCTION_ARGS)
{
	MemoryContext agg_context;
	int				idx = 0;
	mdstate* state;
	TimestampTz* data;

	if (!AggCheckCallContext(fcinfo, &agg_context))
		elog(ERROR, "median_finalfn called in non-aggregate context");

	if (PG_ARGISNULL(0))
		PG_RETURN_NULL();

	state = (mdstate*)PG_GETARG_POINTER(0);
	data = (TimestampTz*)state->data;

	qsort(state->data, state->nelem, sizeof(TimestampTz), &(timestamp_cmp_internal));

	int mid = (state->nelem) / 2;

	if (state->nelem % 2 == 1) {
		TimestampTz v1 = data[mid];
		PG_RETURN_TIMESTAMPTZ(v1);
	}
	else { /* TODO : handle in case of even number of rows */
		TimestampTz v1 = (data[mid] + data[mid - 1]) / 2.0;
		PG_RETURN_TIMESTAMPTZ(v1);
	}
}

