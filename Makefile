SHELL = /bin/bash

CC = gcc
CFLAGS = -std=gnu17 -O3 -march=native -I include -fopenmp -DNDEBUG

OBJ_KERNEL = main.o graph.o reductions.o tiny_solver.o \
			 clique.o unconfined.o neighborhood.o domination.o \
			 single_edge.o extended_single_edge.o twin.o extended_twin.o heavy_vertex.o

OBJ_KERNEL := $(addprefix bin/, $(OBJ_KERNEL))
DEP = $(OBJ_KERNEL)
DEP := $(sort $(DEP))

vpath %.c src src/reductions
vpath %.h include

all : KERNEL

-include $(DEP:.o=.d)

KERNEL : $(OBJ_KERNEL)
	$(CC) $(CFLAGS) -o $@ $^ -lm

bin/%.o : %.c
	$(CC) $(CFLAGS) -MMD -c $< -o $@

.PHONY : clean
clean :
	rm -f KERNEL $(DEP) $(DEP:.o=.d)
