###############################################################
## Basic makefile template.
## Uses google libraries to build test and benchmark suites.
###############################################################

######################## USER CONF ############################

# Define the objects to build (algorithm.cpp -> obj/algorithm.o)
modules = graph generators
# Standard compiler flags.
cppflags = -O3 -Wall -Wextra -pedantic -fPIC --std=c++17
# Additional for the benchmark and release builds.
opt_cppflags = $(cppflags) -DNDEBUG
# Any external libraries to link (gtest/gbench included below).
libs = -ligraph

######################## AUTO CONF ############################

objects = $(addprefix obj/, $(addsuffix .o, $(modules)))
opt_objects = $(addprefix obj/, $(addsuffix .opt.o, $(modules)))

all: bin obj bin/test

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
	valgrind bin/test

# Object files.
obj/%.o: %.cpp
	g++ $(cppflags) -o $@ $< -c

obj/%.opt.o: %.cpp
	g++ $(opt_cppflags) -o $@ $< -c

# Header deps.
obj/graph.o: graph.hpp utils.hpp
obj/graph.opt.o: graph.hpp utils.hpp
obj/test.o: graph.hpp utils.hpp generators.hpp
