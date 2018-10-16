''' Script to reproduce the instance space reported in the graph colouring
instance generation paper. '''

import json

import pandas as pd
import numpy as np
import seaborn as sns
import matplotlib.pyplot as plt
sns.set()

# Load feature data as a pandas dataframe.
with open('evolved-instances-results.json') as infile:
    data = json.load(infile)
df = (
    pd.DataFrame({
        instance['file_name']: instance['features']
        for instance in data})
    .transpose()
    .dropna(axis='index', how='any').astype('float')
)

# Calculate size-normalised features (although this data set is 100 node graphs).
# Algebraic connectivity is scaled by number of vertices, V.
# Energy is scaled by [ V * sqrt(V - 1) ].
df['algebraic_connectivity_normalised'] = df.algebraic_connectivity / df.vertices
df['mean_energy_normalised'] = df.energy / (df.vertices.multiply(df.vertices.sub(1).pow(0.5)))

# Produce plots of features relative to graph density.
sns.pairplot(
    data=df, x_vars=['density'], size=6,
    y_vars=['algebraic_connectivity_normalised', 'mean_energy_normalised']);
plt.savefig('feature_values.png')

# Produce projected instance space as given in the graph generation paper.

# Log-normalise and standardise each feature.
instance_df = df[['density', 'algebraic_connectivity_normalised', 'mean_energy_normalised']]
instance_df = (instance_df - instance_df.min()).add(0.1).apply(np.log)
instance_df = instance_df.subtract(instance_df.mean()).divide(instance_df.std())

# Apply PCA calculated projection.
transform_matrix = np.matrix([
    [0.638, 0.604, 0.478],
    [-0.211, -0.460, 0.862]])
projection = np.asarray(np.matrix(instance_df.values) * transform_matrix.transpose())
instance_df['v1'] = projection[:, 0]
instance_df['v2'] = projection[:, 1]

# Produce plot of the instance space.
sns.lmplot(data=instance_df, x='v1', y='v2', fit_reg=False);
plt.savefig('instance_space.png')
