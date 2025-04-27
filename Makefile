SHELL = /bin/bash

CC = gcc
CFLAGS = -g -std=gnu17 -O3 -march=native -I include -I include/reductions

OBJ_REDUCE = main.o graph.o reducer.o reduction.o degree_zero.o degree_one.o domination.o

OBJ_REDUCE := $(addprefix bin/, $(OBJ_REDUCE))
DEP = $(OBJ_REDUCE)
DEP := $(sort $(DEP))

vpath %.c src src/reductions
vpath %.h include include/reductions

all : REDUCE

-include $(DEP:.o=.d)

REDUCE : $(OBJ_REDUCE)
	$(CC) $(CFLAGS) -o $@ $^

bin/%.o : %.c
	$(CC) $(CFLAGS) -MMD -c $< -o $@

.PHONY : clean
clean :
	rm -f REDUCE $(DEP) $(DEP:.o=.d)
