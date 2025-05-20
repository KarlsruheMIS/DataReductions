SHELL = /bin/bash

CC = gcc
CFLAGS = -g -std=gnu17 -O3 -I include -I include/reductions

OBJ_SHARED = graph.o reducer.o reduction.o \
			 degree_zero.o degree_one.o domination.o neighborhood_removal.o twin.o \
			 triangle.o v_shape.o simultaneous_set.o unconfined.o simplicial_vertex.o \
			 simplicial_vertex_with_weight_transfer.o critical_set.o extended_struction.o \
			 extended_domination.o weighted_funnel.o

OBJ_REDUCE = $(OBJ_SHARED) main.o
OBJ_VIS = $(OBJ_SHARED) visualizer.o main_visualizer.o
OBJ_LIB = $(OBJ_SHARED) mwis_reductions.o

OBJ_REDUCE := $(addprefix bin/, $(OBJ_REDUCE))
OBJ_VIS := $(addprefix bin/, $(OBJ_VIS))
OBJ_LIB := $(addprefix bin/, $(OBJ_LIB))
DEP = $(OBJ_REDUCE) $(OBJ_LIB)
DEP := $(sort $(DEP))

vpath %.c src src/reductions
vpath %.h include include/reductions

all : mwis_reduce mwis_visualize libmwis_reductions.a

-include $(DEP:.o=.d)

mwis_reduce : $(OBJ_REDUCE)
	$(CC) $(CFLAGS) -o $@ $^

mwis_visualize : $(OBJ_VIS)
	$(CC) $(CFLAGS) -fopenmp -o $@ $^ -lm -lglut -lGLU -lGL

libmwis_reductions.a : $(OBJ_LIB)
	ar r $@ $^

bin/%.o : %.c
	$(CC) $(CFLAGS) -MMD -c $< -o $@

.PHONY : clean
clean :
	rm -f mwis_reduce mwis_visualize libmwis_reductions.a $(DEP) $(DEP:.o=.d)
