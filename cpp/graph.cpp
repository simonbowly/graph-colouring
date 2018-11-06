
#include <cmath>
#include <fstream>
#include <gsl/gsl_assert>
#include "graph.hpp"
#include <iostream>

using namespace std;
using namespace graph;


void UndirectedGraph::add_edges(vector<pair<int, int>> edges)
{
    igraph_vector_t v;
    igraph_vector_init(&v, edges.size() * 2);
    int i = 0;
    for (auto edge : edges) {
        VECTOR(v)[i] = edge.first;
        VECTOR(v)[i+1] = edge.second;
        i += 2;
    }
    igraph_add_edges(graph.get(), &v, 0);
    igraph_vector_destroy(&v);
}

IGraphVector UndirectedGraph::degree() const {
    igraph_vector_t v;
    igraph_vector_init(&v, vertices());
    /*int ret = */igraph_degree(graph.get(), &v, igraph_vss_all(), IGRAPH_ALL, IGRAPH_NO_LOOPS);
    auto res = IGraphVector(v);
    Ensures(res.size() == vertices());
    return res;
}

IGraphVector UndirectedGraph::betweenness_centrality() const {
    igraph_vector_t v;
    igraph_vector_init(&v, vertices());
    /*int ret = */igraph_betweenness(
        graph.get(), &v,
        igraph_vss_all(),
        false,          // false = undirected
        nullptr,        // NULL = unweighted
        true);          // nobigint = true
    auto res = IGraphVector(v);
    Ensures(res.size() == vertices());
    return res;
}

IGraphVector UndirectedGraph::eigenvector_centrality() const {
    igraph_vector_t v;
    igraph_vector_init(&v, vertices());
    igraph_arpack_options_t options;
    igraph_arpack_options_init(&options);
    /*int ret = */igraph_eigenvector_centrality(
        graph.get(), &v,
		nullptr,        // eigenvalue (not needed)
        false,          // false = undirected
        false,          // false = do not scale
        nullptr,        // nul = unweighted
        &options);
    auto res = IGraphVector(v);
    Ensures(res.size() == vertices());
    return res;
}

IGraphVector UndirectedGraph::adjacency_eigenvalues() const {
    // Get adjacency matrix.
    igraph_matrix_t adjacency;
    igraph_matrix_init(&adjacency, vertices(), vertices());
    /*int ret = */igraph_get_adjacency(
        graph.get(), &adjacency,
        IGRAPH_GET_ADJACENCY_BOTH,  // upper and lower triangular
        false);                     // false = number of edges

    // Calculate all eigenvalues.
    igraph_vector_t v;
    igraph_vector_init(&v, vertices());
    /*int ret = */igraph_lapack_dsyevr(
        &adjacency,
		IGRAPH_LAPACK_DSYEV_ALL,
		0.0, 0.0, 0.0,      // bounds for eigenvalues (only for INTERVAL mode)
		0, 0,               // lower and upper indexing (only for SELECT mode)
        1e-10,              // convergence tolerance
		&v,                 // resulting eigenvalues
        nullptr,            // eigenvectors are discarded
		nullptr);           // support is discarded

    // Cleanup.
    igraph_matrix_destroy(&adjacency);

    auto res = IGraphVector(v);
    Ensures(res.size() == vertices());
    return res;
}


tuple<double, double, double> UndirectedGraph::adjacency_eigenvalue_stats() const {
    // Returns:
    //      energy (mean of absolute values of eigenvalues)
    //      (absolute) eigenvalue standard deviation
    //      beta bipartitivity parameter (even closed walks/all closed walks)

    // Calculate eigenvalues and absolutes.
    auto eigenvalues = adjacency_eigenvalues();
    vector<double> absolute_eigenvalues;
    absolute_eigenvalues.reserve(vertices());
    for (const auto & v : adjacency_eigenvalues()) {
        absolute_eigenvalues.push_back(fabs(v));
    }
    Ensures(absolute_eigenvalues.size() == (uint) vertices());

    // Mean/stdev statistics.
    auto [energy, stdev] = simple_statistics(absolute_eigenvalues);

    // Beta bipartitivity.
    double sc_even = 0.0;
    double sc_total = 0.0;
    for (const double& eig : adjacency_eigenvalues()) {
        sc_even += cosh(eig);
        sc_total += exp(eig);
    }

    return make_tuple(energy, stdev, sc_even / sc_total);
}


double UndirectedGraph::algebraic_connectivity_lapack_dense() const {
    
    // Short-circuit.
    if (!is_connected()) { return 0; }

    // Get laplacian matrix.
    igraph_matrix_t laplacian;
    igraph_matrix_init(&laplacian, vertices(), vertices());
    /*int ret = */igraph_laplacian(
        graph.get(), &laplacian,
        nullptr,                    // don't create sparse laplacian
        false,                      // false = non-normalised
        nullptr);                   // null = unweighted

    // Calculate second smallest eigenvalue.
    igraph_vector_t v;
    // Must allocate N-length workspace to avoid memory issues.
    //      ref https://github.com/igraph/igraph/issues/1109
    igraph_vector_init(&v, vertices());
    /*int ret = */igraph_lapack_dsyevr(
        &laplacian,
		IGRAPH_LAPACK_DSYEV_SELECT,
		0.0, 0.0, 0.0,      // bounds for eigenvalues (only for INTERVAL)
		2, 2,               // select second smallest
        1e-10,              // convergence tolerance
		&v,                 // resulting eigenvalues
        nullptr,            // eigenvectors are discarded
		nullptr);           // support is discarded

    // Cleanup.
    igraph_matrix_destroy(&laplacian);

    // Resulting vector has size 1, so begin() points to the result.
    auto result = IGraphVector(v);
    Ensures(result.size() == 1);
    return *result.begin();
}

int multiplier(igraph_real_t *to, const igraph_real_t *from, int n, void *extra) {

    igraph_matrix_t* A = (igraph_matrix_t*) extra;
    igraph_vector_t row;
    igraph_vector_init(&row, n);

    for (int i = 0; i < n; i++) {
        igraph_matrix_get_row(A, &row, i);
        to[i] = 0.0;
        for (int j = 0; j < n; j++) {
            to[i] += VECTOR(row)[j] * from[j];
        }
    }

    igraph_vector_destroy(&row);

    return 0;
}

double UndirectedGraph::algebraic_connectivity_arpack_dense() const {

    // Short-circuit.
    if (!is_connected()) { return 0; }

    // Get laplacian matrix.
    igraph_matrix_t laplacian;
    igraph_matrix_init(&laplacian, vertices(), vertices());
    /*int ret = */igraph_laplacian(
        graph.get(), &laplacian,
        nullptr,                    // don't create sparse laplacian
        false,                      // false = non-normalised
        nullptr);                   // null = unweighted

    // ARPACK configuration for eigenvalue calculation.
    igraph_arpack_options_t options;
    igraph_arpack_options_init(&options);
    options.n = vertices();
    options.which[0]='S'; options.which[1]='A';     // calculate from the small end
    options.nev = 2;                                // get two smallest values
    options.ncv = 0;                                // 0 means "automatic" in igraph_arpack_rssolve
    options.start = 0;		                        // random start vector
    options.mxiter = 10000;                         // iterations to convergence

    // Callback eigenvalue calculation.
    igraph_vector_t values;
    igraph_vector_init(&values, 0);
    igraph_arpack_rssolve(
        multiplier, &laplacian,     // Callback multiplying L * x
        &options,
        nullptr,                    // Automatic storage structures.
        &values,                    // Eigenvalues.
        nullptr);                   // Eigenvectors not required.

    // Cleanup.
    igraph_matrix_destroy(&laplacian);

    // Resulting vector has size 2, so begin() + 1 points to the result.
    auto result = IGraphVector(values);
    Ensures(result.size() == 2);
    return *(result.begin() + 1);

}


double UndirectedGraph::wiener_index() const {
    // Simple sum of inter-vertex distances over unordered vertex pairs.
    igraph_matrix_t res;
    igraph_matrix_init(&res, vertices(), vertices());
    igraph_shortest_paths(graph.get(), &res, igraph_vss_all(), igraph_vss_all(), IGRAPH_ALL);

    double result = 0;
    for (int i = 0; i < vertices(); i++) {
        for (int j = i + 1; j < vertices(); j++) {
            result += MATRIX(res, i, j);
        }
    }

    igraph_matrix_destroy(&res);
    return result;
}


pair<double, double> UndirectedGraph::szeged_indices() const {
    // Sum of n(u;v)n(v;u) over all edges.

    // Edge list.
    igraph_vector_t res;
    igraph_vector_init(&res, edges() * 2);
    igraph_get_edgelist(graph.get(), &res, false);
    auto edge_list = IGraphVector(res);

    // Distance matrix (needs cleanup).
    igraph_matrix_t distance;
    igraph_matrix_init(&distance, vertices(), vertices());
    igraph_shortest_paths(graph.get(), &distance, igraph_vss_all(), igraph_vss_all(), IGRAPH_ALL);

    double szeged = 0, revised_szeged = 0;

    for (int e = 0; e < edges(); e++) {

        int u = edge_list[e * 2];
        int v = edge_list[e * 2 + 1];
        double n_uv = 0, n_vu = 0, o_uv = 0;

        // Compare distances from all other vertices to u and v.
        for (int i = 0; i < vertices(); i++) {
            if ((i == u) || (i == v)) { continue; }

            if (MATRIX(distance, u, i) < MATRIX(distance, v, i)) {
                // vertex i is closer to u than v
                n_uv += 1;
            } else if (MATRIX(distance, u, i) > MATRIX(distance, v, i)) {
                // vertex i is closer to v than u
                n_vu += 1;
            } else {
                // i is equidistant from v and u
                o_uv += 1;
            }
        }

        szeged += n_uv * n_vu;
        revised_szeged += (n_uv + o_uv / 2) * (n_vu + o_uv / 2);

    }

    igraph_matrix_destroy(&distance);
    return make_pair(szeged, revised_szeged);
}


double UndirectedGraph::average_path_length() const {
    igraph_real_t res;
    /*int ret = */igraph_average_path_length(
        graph.get(), &res,
        false,          // igraph_bool_t directed (ignored)
        true);          // igraph_bool_t unconn
    return res;
}

int UndirectedGraph::diameter() const {
    igraph_integer_t res;
    /*int ret = */igraph_diameter(
        graph.get(), &res,
        nullptr,        // igraph_integer_t *pfrom
        nullptr,        // igraph_integer_t *pto
        nullptr,        // igraph_vector_t *path
        false,          // igraph_bool_t directed (ignored)
        true);          // igraph_bool_t unconn
    return res;
}

int UndirectedGraph::radius() const {
    igraph_real_t res;
    /*int ret = */igraph_radius(
        graph.get(), &res,
        IGRAPH_ALL);    // igraph_neimode_t mode
    return res;
}

int UndirectedGraph::girth() const {
    igraph_integer_t res;
    /*int ret = */igraph_girth(
        graph.get(), &res,
        nullptr);       // igraph_vector_t *circle
    return res;
}

double UndirectedGraph::clustering_coefficient() const {
    igraph_real_t res;
    /*int ret = */igraph_transitivity_undirected(
        graph.get(), &res,
        IGRAPH_TRANSITIVITY_ZERO);  // igraph_transitivity_mode_t mode
    return res;
}

IGraphVSIterator UndirectedGraph::neighbours(int v) const {
    igraph_vs_t vs;
    igraph_vit_t vit;
    igraph_vs_adj(&vs, v, IGRAPH_OUT);
    igraph_vit_create(graph.get(), vs, &vit);
    return IGraphVSIterator(vs, vit);
}


vector<IGraphVector> igraph_vector_ptr_extract(igraph_vector_ptr_t& p) {
    vector<IGraphVector> r;
    int n = igraph_vector_ptr_size(&p);
    for (int i = 0; i < n; i++) {
        igraph_vector_t* v = (igraph_vector_t*) igraph_vector_ptr_e(&p, i);
        IGraphVector indep_set(*v);
        r.push_back(move(indep_set));
        igraph_free(v);
    }
    igraph_vector_ptr_destroy(&p);
    return r;
}


vector<IGraphVector> UndirectedGraph::maximal_independent_vertex_sets() const {
    igraph_vector_ptr_t p;
    igraph_vector_ptr_init(&p, 0);
    igraph_maximal_independent_vertex_sets(graph.get(), &p);
    return igraph_vector_ptr_extract(p);
}


vector<IGraphVector> UndirectedGraph::maximal_cliques() const {
    igraph_vector_ptr_t p;
    igraph_vector_ptr_init(&p, 0);
    igraph_maximal_cliques(graph.get(), &p, 0, 0);
    return igraph_vector_ptr_extract(p);
}


UndirectedGraph graph::read_dimacs(string file_name) {

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


UndirectedGraph graph::random_tree(int vertices, int children) {
    auto g = graph::impl::create_igraph_ptr();
    igraph_tree(g.get(), vertices, children, IGRAPH_TREE_UNDIRECTED);
    return UndirectedGraph(g);
}


UndirectedGraph graph::random_bipartite(int n1, int n2, double p) {
    auto g = graph::impl::create_igraph_ptr();
    igraph_bipartite_game(g.get(), nullptr, IGRAPH_ERDOS_RENYI_GNP, n1, n2, p, 0, false, IGRAPH_ALL);
    return UndirectedGraph(g);
}


UndirectedGraph graph::erdos_renyi_gnm(int n, int m) {
    auto g = graph::impl::create_igraph_ptr();
    igraph_erdos_renyi_game(g.get(), IGRAPH_ERDOS_RENYI_GNM, n, m, false, false);
    return UndirectedGraph(g);
}


UndirectedGraph graph::erdos_renyi_gnp(int n, double p) {
    auto g = graph::impl::create_igraph_ptr();
    igraph_erdos_renyi_game(g.get(), IGRAPH_ERDOS_RENYI_GNP, n, p, false, false);
    return UndirectedGraph(g);
}
