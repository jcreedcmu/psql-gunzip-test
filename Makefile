MODULES = func

PG_CONFIG = pg_config
PGXS = $(shell $(PG_CONFIG) --pgxs)
INCLUDEDIR = $(shell $(PG_CONFIG) --includedir-server)
include $(PGXS)

func.so: func.o
	gcc -shared -o func.so func.o -lz

func.o: func.c
	gcc  -o func.o -c func.c  $(CFLAGS) -I$(INCLUDEDIR)
