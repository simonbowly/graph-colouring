''' Calculate simple features for graph colouring instances. '''

import scipy.sparse
import numpy as np


def read_dimacs(file_name):
    '''
    Read a DIMACS format graph file, returning the number of vertices and
    set of undirected edges as a dictionary. DIMACS format is as follows:

        c File: data_files/evolved_graphs/g0001.col
        p edge 100 1902
        e 1 2
        e 1 6
        e 1 10
        e 1 11
        e 1 18
        ...

    Where 'c' comment lines are ignored, 'p' line specifies graph size,
    'e' lines specify undirected edges.
    '''
    with open(file_name) as infile:
        while True:
            line = next(infile)
            if line.startswith('p'):
                _, _, n_vertices, n_edges = line.split()
                n_vertices = int(n_vertices)
                n_edges = int(n_edges)
                break
        edges = (
            edge.replace('e', '').strip().split()
            for edge in infile
            if edge.startswith('e'))
        edges = set(
            (int(i), int(j))
            for i, j in edges)
    if len(edges) != n_edges:
        raise ValueError(
            f"Number of edges specified in graph file ({len(edges)}) "
            f"does not match declared value in header ({n_edges}).")
    return dict(n_vertices=n_vertices, edges=edges)


def adjacency_matrix(n_vertices, edges):
    ''' Return a sparse adjacency matrix for the given graph data. '''
    data = [1 for _ in edges]
    i, j = zip(*((a-1, b-1) for a, b in edges))
    adj = scipy.sparse.coo_matrix((data, (i, j)), (n_vertices, n_vertices))
    return adj + adj.transpose()


def laplacian_matrix(adj):
    ''' Return a sparse laplacian matrix for the given adjacency matrix. '''
    n_vertices = adj.shape[0]
    data = np.asarray(adj.sum(axis=0))[0]
    i = np.arange(n_vertices)
    degree_mat = scipy.sparse.coo_matrix((data, (i, i)), (n_vertices, n_vertices))
    return degree_mat - adj


def graph_features(n_vertices, edges):
    ''' Return a simple feature set for the given graph data. '''
    n_edges = len(edges)
    adj = adjacency_matrix(n_vertices, edges)
    laplacian = laplacian_matrix(adj)
    return {
        'vertices': int(n_vertices),
        'edges': int(n_edges),
        'density': float(2 * n_edges / (n_vertices * (n_vertices - 1))),
        # Sum of magnitudes of eigenvalues of the adjacency matrix.
        'energy': float(np.abs(np.linalg.eigvals(adj.todense())).sum()),
        # Second smallest eigenvalue of the laplacian matrix.
        'algebraic_connectivity': float(np.sort(np.abs(np.linalg.eigvals(laplacian.todense())))[1]),
    }


if __name__ == '__main__':
    # Run a simple test when this file is run as a script.
    print(graph_features(**read_dimacs('test_case.col')))
