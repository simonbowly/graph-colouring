
#include <iostream>

#include "graph.hpp"
#include "generators.hpp"

using namespace std;
using namespace graph;


int main() {

    default_random_engine rng;

    // const UndirectedGraph g = erdos_renyi(rng, 30, 0.8);
    const UndirectedGraph g = read_dimacs("../test_case.col");

    // Basic properties.
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
    cout << "12. Szeged index:          " << endl;
    cout << "13. Beta:                  " << endl;
    tie( mean, stdev ) = g.abs_adjacency_eigenvalues_statistics();
    cout << "14. Energy:                " << mean << endl;
    tie( mean, stdev ) = g.adjacency_eigenvalues_statistics();
    cout << "15. Eigenvalue StDev:      " << stdev << endl;
    cout << "16. Alg. Connectivity:     " << g.algebraic_connectivity() << endl;
    tie( mean, stdev ) = g.eigenvector_centrality_statistics();
    cout << "17. E Centrality Mean:     " << mean << endl;
    cout << "18. E Centrality StDev:    " << stdev << endl;

    return 0;

}
