'''
Script to evaluate the evolved data set of 100 node graphs. Generic error
handling and logging added to avoid algorithm timeouts and errors in reading
instance files for feature calculation.

Run this script to reproduce evolved-instances-results.json which will contain
feature and algorithm performance data for evolved instances.
'''

import glob
import json
import multiprocessing
import logging

import tqdm

from features import graph_features, read_dimacs
from algorithms import run_maxis, run_dsatur


def calculate_features(file_name):
    try:
        graph = read_dimacs(file_name)
        return graph_features(**graph)
    except Exception as e:
        logging.warning(f"Evaluating features failed for {file_name} ({e}).")
        return None


def solve_dsatur_timeout(file_name):
    try:
        return run_dsatur(file_name, timeout_seconds=10)
    except Exception as e:
        logging.warning(f"Evaluating DSATUR failed for {file_name} ({e}).")
        return None


def solve_maxis_timeout(file_name):
    try:
        return run_maxis(file_name, timeout_seconds=10)
    except Exception as e:
        logging.warning(f"Evaluating MAXIS failed for {file_name} ({e}).")
        return None


def evaluate(file_name):
    ''' For a given instance file, calculate features and run heuristics.
    In case of an error in any evaluation, the error message is logged
    and the relevant field will be stored as None. '''
    return dict(
        file_name=file_name,
        features=calculate_features(file_name),
        algorithms=dict(
            dsatur=solve_dsatur_timeout(file_name),
            maxis=solve_maxis_timeout(file_name)),
        )


# Use multiprocessing to evaluate the complete instance set, write results to JSON file.

file_names = sorted(glob.glob('graphs/instances/g*.col'))
pool = multiprocessing.Pool()
results = list(tqdm.tqdm(
    pool.imap_unordered(evaluate, file_names),
    total=len(file_names)))

with open('evolved-instances-results-tmp.json', 'w') as outfile:
    json.dump(results, outfile)
