
#include <iostream>

#include "graph.hpp"

using namespace std;
using namespace graph;


void print_features(const UndirectedGraph& g) {
    cout << " 1. Vertices:              " << g.vertices() << endl;
    cout << " 2. Edges:                 " << g.edges() << endl;
    cout << " 3. Density:               " << density(g) << endl;
    auto [ mean, stdev ] = simple_statistics(degree(g));
    cout << " 4. Degree Mean:           " << mean << endl;
    cout << " 5. Degree StDev:          " << stdev << endl;
    cout << " 6. Average Path Length:   " << average_path_length(g) << endl;
    cout << " 7. Diameter:              " << diameter(g) << endl;
    cout << " 8. Girth:                 " << girth(g) << endl;
    tie( mean, stdev ) = simple_statistics(betweenness_centrality(g));
    cout << " 9. B Centrality Mean:     " << mean << endl;
    cout << "10. B Centrality StDev:    " << stdev << endl;
    cout << "11. Clustering Coeff:      " << clustering_coefficient(g) << endl;
    auto [ szeged, revised_szeged ] = szeged_indices(g);
    cout << "12. Szeged Index:          " << szeged << endl;
    cout << "    Revised Szeged Index:  " << revised_szeged << endl;
    auto [ energy, eig_stdev, beta ] = adjacency_eigenvalue_stats(g);
    cout << "13. Beta:                  " << beta << endl;
    cout << "14. Energy:                " << energy << endl;
    cout << "15. Eigenvalue StDev:      " << eig_stdev << endl;
    cout << "16. Alg. Connectivity:     " << algebraic_connectivity_lapack_dense(g) << endl;
    cout << "    Alg. Connectivity:     " << algebraic_connectivity_arpack_dense(g) << endl;
    tie( mean, stdev ) = simple_statistics(eigenvector_centrality(g));
    cout << "17. E Centrality Mean:     " << mean << endl;
    cout << "18. E Centrality StDev:    " << stdev << endl;
}


int main() {

    // Example graph construction by edges.
    cout << "========= TINY ========" << endl;
    // Set number of vertices N.
    auto g = UndirectedGraph(5);
    // Add edges as an adjacency list (numbering is 0 .. N-1).
    // (i, j) ordering doesn't matter, but be aware that igraph
    // will count edges multiple times if they are added more than
    // once in any order (TO FIX).
    vector<pair<int, int>> edges;
    edges.emplace_back(0, 1);
    edges.emplace_back(1, 2);
    edges.emplace_back(2, 3);
    edges.emplace_back(3, 4);
    edges.emplace_back(4, 0);
    g.add_edges(edges);
    print_features(g);

    // Calculate features of some generated graphs.
    cout << "========= TREE ========" << endl;
    g = random_tree(100, 10);
    print_features(g);
    cout << "====== BIPARTITE ======" << endl;
    g = random_bipartite(20, 30, 0.3);
    print_features(g);
    cout << "===== ERDOS-RENYI =====" << endl;
    g = erdos_renyi_gnp(10, 0.9);
    print_features(g);

    return 0;

}
