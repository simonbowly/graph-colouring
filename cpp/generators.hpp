
#ifndef GENERATORS_HPP
#define GENERATORS_HPP

#include <random>
#include <string>

#include "igraph/igraph.h"

#include "graph.hpp"


namespace graph {

    UndirectedGraph erdos_renyi(std::default_random_engine& rng, int n, double p);
    UndirectedGraph read_dimacs(std::string);

}

#endif
