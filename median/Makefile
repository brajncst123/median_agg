MODULE_big = median
EXTENSION = median
DATA = median--1.0.sql
DOCS = README.md
REGRESS := median
PG_USER = postgres
REGRESS_OPTS := \
	--load-extension=$(EXTENSION) \
	--user=$(PG_USER) \
	--inputdir=test \
	--outputdir=test \
	--temp-instance=${PWD}/tmpdb

SRCS = median.c
OBJS = $(patsubst %.c,%.o,$(SRCS))

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)

