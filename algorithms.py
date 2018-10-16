'''
Runs the Culberson heuristic algorithms. Each compiled algorithm is run
as a subprocess with possible timeout. Parameters accepted by the algorithm
executables are currently hard-coded and passed via stdin.

Note that when each algorithm runs it appends its result to a .res file
in the same path as the input file (as well as writing the result to
stdout which is captured by these functions).

Algorithm code and manual for use is available here:
http://webdocs.cs.ualberta.ca/~joe/Coloring/Colorsrc/index.html
'''

import os
import re
import random
from subprocess import check_output, TimeoutExpired, CalledProcessError


def run_culberson_algorithm(algorithm, instance_file, algorithm_options, *,
                            seed=None, timeout_seconds=None):
    ''' Run :executable on the given :instance_file. Prepend options (cheat and seed)
    used by all Culberson algorithms to the given list of :algorithm_options. '''
    executable = os.path.join(os.path.dirname(__file__), 'color', 'bin', algorithm)
    standard_options = [
        '0',                                                    # don't use cheat
        str(random.getrandbits(16) if seed is None else seed),  # random seed if none provided
    ]
    return check_output(
        [executable, instance_file],
        input='\n'.join(standard_options + algorithm_options),
        universal_newlines=True,
        timeout=timeout_seconds)


def read_coloring_output(output):
    ''' Interpret Culberson algorithm output, returning a dictionary with keys:
    'colors', 'color_sum', 'cpu_time'. '''
    match = re.search('CLRS\s*=\s*([0-9]+)\s*CLRSUM\s*=\s*([0-9]+)', output)
    result = dict(zip(['colors', 'color_sum'], (int(i) for i in match.groups())))
    match = re.search('Coloring time cpu\s*=\s*([0-9\.]+)', output)
    result['cpu_time'] = float(match.group(1))
    return result


def run_dsatur(file_name, **kwargs):
    ''' Run DSATUR on a DIMACS file. If :timeout_seconds is given and the
    specified time limit is exceeded, this function will raise a
    subprocess.TimeoutExpired exception.

    Explanation of parameters can be found here:
    http://webdocs.cs.ualberta.ca/~joe/Coloring/Colorsrc/manual.html#dsatur '''
    options = [
        '3',            # mode setting - decreasing degree
    ]
    output = run_culberson_algorithm('dsatur', file_name, options, **kwargs)
    return read_coloring_output(output)


def run_maxis(file_name, **kwargs):
    ''' Run MAXIS on a DIMACS file. If :timeout_seconds is given and the
    specified time limit is exceeded, this function will raise a
    subprocess.TimeoutExpired exception.

    Explanation of parameters can be found here:
    http://webdocs.cs.ualberta.ca/~joe/Coloring/Colorsrc/manual.html#maxis
    '''
    options = [
        '0 3',          # basic branching strategy
        '0 75 50',      # no backtrack limits
    ]
    output = run_culberson_algorithm('maxis', file_name, options, **kwargs)
    return read_coloring_output(output)


if __name__ == '__main__':
    # Run a simple test when this file is run as a script.
    print(run_dsatur('test_case.col'))
    print(run_maxis('test_case.col'))
