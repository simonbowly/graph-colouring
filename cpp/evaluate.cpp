
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
    tie( mean, stdev ) = simple_statistics(eigenvector_centrality(g));
    cout << "17. E Centrality Mean:     " << mean << endl;
    cout << "18. E Centrality StDev:    " << stdev << endl;
}


int main(int argc, char *argv[]) {

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
