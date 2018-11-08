
import json

import pandas as pd


with open('lewis-evolved-results.json') as infile:
    data = json.load(infile)


df = pd.DataFrame(
    [
        dict(instance=instance, metric=key, value=metrics[key], algorithm=algorithm)
        for instance, results in data
        for algorithm, metrics in results.items()
        if metrics is not None
        for key in ['colours', 'time_ms', 'constraint_checks']
    ]
).set_index(['instance', 'algorithm', 'metric']).value.unstack().unstack()


# Errors
print('==== Errors ====')
for instance, results in data:
    for algorithm, metrics in results.items():
        if metrics is None:
            print(instance, algorithm)

print('==== Data ======')
print(df.colours.head())
print(df.time_ms.head())

df.to_excel('evolved-algorithms.xlsx')
