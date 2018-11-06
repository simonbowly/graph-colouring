
#include <iostream>
#include <fstream>
#include <utility>
#include <vector>

#include "generators.hpp"

using namespace std;


graph::UndirectedGraph graph::erdos_renyi(default_random_engine& rng, int n, double p) {

    geometric_distribution<int> dist(p);
    vector<pair<int, int>> edges;

    int v = 1, w = -1;
    while (v < n) {
        w = w + 1 + dist(rng);
        while ((w >= v) && (v < n)) {
            w = w - v;
            v = v + 1;
        }
        if (v < n) {
            edges.push_back(make_pair(v, w));
        }
    }

    graph::UndirectedGraph graph(n);
    graph.add_edges(edges);
    return graph;
}


graph::UndirectedGraph graph::read_dimacs(string file_name) {

    uint vertices = 0, edges = 0;
    vector<pair<int, int>> edge_list;

    string line;
    ifstream col_file(file_name);
    if (col_file.is_open()) {
        while ( getline(col_file, line) ) {
            if (line.substr(0, 1).compare("p") == 0) {
                // assert vertices, edges are 0
                string info = line.substr(7, line.size());
                auto found = info.find(" ");
                vertices = stoi(info.substr(0, found));
                edges = stoi(info.substr(found + 1, info.size()));
                edge_list.reserve(edges);
            } else if (line.substr(0, 1).compare("e") == 0) {
                // assert vertices, edges are not 0
                string info = line.substr(2, line.size());
                auto found = info.find(" ");
                int a = stoi(info.substr(0, found));
                int b = stoi(info.substr(found + 1, info.size()));
                edge_list.emplace_back(a - 1, b - 1);
            }
        }
    } else {
        throw "File not open.";
    }

    if (edge_list.size() != edges) {
        throw "Incorrect number of edges.";
    }

    graph::UndirectedGraph g(vertices);
    g.add_edges(edge_list);
    return g;

}
