
#include <cmath>
#include <gsl/gsl_assert>
#include "graph.hpp"

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

vector<double> UndirectedGraph::abs_adjacency_eigenvalues() const {
    vector<double> result;
    result.reserve(vertices());
    for (const auto & v : adjacency_eigenvalues()) {
        result.push_back(fabs(v));
    }
    Ensures(result.size() == (uint) vertices());
    return result;
}

double UndirectedGraph::algebraic_connectivity() const {
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
    igraph_vector_init(&v, 1);
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
