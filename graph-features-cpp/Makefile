###############################################################
## Basic makefile template.
## Uses google libraries to build test and benchmark suites.
###############################################################

######################## USER CONF ############################

# Define the objects to build (algorithm.cpp -> obj/algorithm.o)
modules = graph
# Standard compiler flags.
cppflags = -O3 -Wall -Wextra -pedantic -fPIC --std=c++17
# Additional for the benchmark and release builds.
opt_cppflags = $(cppflags) -DNDEBUG
# Any external libraries to link (gtest/gbench included below).
libs = -ligraph

######################## AUTO CONF ############################

objects = $(addprefix obj/, $(addsuffix .o, $(modules)))
opt_objects = $(addprefix obj/, $(addsuffix .opt.o, $(modules)))

all: bin obj bin/test bin/evaluate

run: test memtest bench

clean:
	rm -f obj/*
	rm -f bin/*

bin:
	mkdir -p bin

obj:
	mkdir -p obj

# Tests run with standard objects (i.e. asserts on).
bin/test: $(objects) obj/test.o
	g++ -o bin/test $(objects) obj/test.o $(libs) -lgtest -lpthread

test: bin/test
	@bin/test

# Memory access error and leak checking
memtest: bin/test
	valgrind --leak-check=full bin/test

# Executable to evaluate instances.
bin/evaluate: $(opt_objects) obj/evaluate.opt.o
	g++ -o bin/evaluate $(opt_objects) obj/evaluate.opt.o $(libs)

# Object files.
obj/%.o: %.cpp
	g++ $(cppflags) -o $@ $< -c

obj/%.opt.o: %.cpp
	g++ $(opt_cppflags) -o $@ $< -c

# Header deps.
obj/graph.o: graph.hpp utils.hpp
obj/graph.opt.o: graph.hpp utils.hpp
obj/test.o: graph.hpp utils.hpp
