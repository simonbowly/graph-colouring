
#include <iostream>

#include "graph.hpp"

using namespace std;
using namespace graph;


void print_features(const UndirectedGraph& g) {
    cout << " 1. Vertices:              " << g.vertices() << endl;
    cout << " 2. Edges:                 " << g.edges() << endl;
    cout << " 3. Density:               " << g.density() << endl;
    auto [ mean, stdev ] = g.degree_statistics();
    cout << " 4. Degree Mean:           " << mean << endl;
    cout << " 5. Degree StDev:          " << stdev << endl;
    cout << " 6. Average Path Length:   " << g.average_path_length() << endl;
    cout << " 7. Diameter:              " << g.diameter() << endl;
    cout << " 8. Girth:                 " << g.girth() << endl;
    tie( mean, stdev ) = g.betweenness_centrality_statistics();
    cout << " 9. B Centrality Mean:     " << mean << endl;
    cout << "10. B Centrality StDev:    " << stdev << endl;
    cout << "11. Clustering Coeff:      " << g.clustering_coefficient() << endl;
    auto [ szeged, revised_szeged ] = g.szeged_indices();
    cout << "12. Szeged Index:          " << szeged << endl;
    cout << "    Revised Szeged Index:  " << revised_szeged << endl;
    auto [ energy, eig_stdev, beta ] = g.adjacency_eigenvalue_stats();
    cout << "13. Beta:                  " << beta << endl;
    cout << "14. Energy:                " << energy << endl;
    cout << "15. Eigenvalue StDev:      " << eig_stdev << endl;
    cout << "16. Alg. Connectivity:     " << g.algebraic_connectivity_lapack_dense() << endl;
    tie( mean, stdev ) = g.eigenvector_centrality_statistics();
    cout << "17. E Centrality Mean:     " << mean << endl;
    cout << "18. E Centrality StDev:    " << stdev << endl;
}


int main(int argc, char *argv[]) {

    if (argc == 1) {

        auto g = read_dimacs("../test_case.col");
        // auto g = random_tree(30, 5);
        // auto g = random_bipartite(20, 30, 0.3);
        // auto g = erdos_renyi_gnp(10, 0.9);
        print_features(g);

    }

    for (int i = 1; i < argc; i++) {
        string instance_file(argv[i]);
        try {
            const UndirectedGraph g = read_dimacs(instance_file);
            cout << "===== " << instance_file << " =====" << endl;
            print_features(g);
        } catch (...) {
            cerr << "Skipped " << instance_file << " due to error" << endl;
        }
    }

    return 0;

}
