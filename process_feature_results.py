'''
Parse the resulting features.out produced by:
    cpp/bin/test graphs/instances/g*.col | tee evolved-features.out | grep === | tqdm > /dev/null
to a dataframe with instances as index and features as columns.
'''

import pathlib
import pandas as pd

results = pd.DataFrame({
    line.partition('\n')[0].strip('= ').rpartition('/')[2].partition('.col')[0]:    # Reduces first line to the path stem as a key.
    {
        a[4:]: float(b.strip())     # Feature name (minus some consistent-sized padding): feature value.
        for a, _, b in (
            entry.partition(':')    # Breaks feature:value.
            for entry in line.strip().split('\n')[1:])      # All but the first line are feature values.
        if b.strip()                # Ignore empty values (pandas will insert NaNs).
    }
    for line in pathlib.Path('evolved-features.out').read_text().split('\n==')      # Splits each file.
}).transpose()


print(results.describe().transpose())


results[[
    'Vertices',
    'Edges',
    'Density',
    'Degree Mean',
    'Degree StDev',
    'Average Path Length',
    'Diameter',
    'Girth',
    'B Centrality Mean',
    'B Centrality StDev',
    'Clustering Coeff',
    'Szeged Index',
    'Revised Szeged Index',
    'Beta',
    'Energy',
    'Eigenvalue StDev',
    'Alg. Connectivity',
    'E Centrality Mean',
    'E Centrality StDev',
]].rename(columns={
    'B Centrality Mean': 'Betweenness Centrality Mean',
    'B Centrality StDev': 'Betweenness Centrality StDev',
    'Alg. Connectivity': 'Algebraic Connectivity',
    'E Centrality Mean': 'Eigenvector Centrality Mean',
    'E Centrality StDev': 'Eigenvector Centrality StDev',
    'Clustering Coeff': 'Clustering Coefficient',
    'Beta': 'Beta Bipartitivity',
}).to_excel('evolved-features.xlsx')
