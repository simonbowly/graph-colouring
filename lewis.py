
import contextlib
import pathlib
import subprocess
import tempfile


@contextlib.contextmanager
def temporary_directory():
    ''' Create a temporary directory cleaned up at the end of the context.
    Pass the directory path to the context as a pathlib.Path object. '''
    try:
        output_dir = tempfile.TemporaryDirectory()
        yield pathlib.Path(output_dir.name)
    finally:
        output_dir.cleanup()


def read_effort_file(text, ncolors, key):
    lines = [
        line.split('\t')
        for line in text.strip().split('\n')]
    # Not always present...?
    if tuple(lines[-1]) == ('1', 'X'):
        lines = lines[:-1]
    # Reads the improvement lines: effort to find each new incumbent.
    improvements = [
        {'colours': int(colours), key: int(constraint_checks)}
        for colours, constraint_checks in lines[:-1]
    ]
    last_line = lines[-1]
    # assert int(last_line[0]) == ncolors - 1 ????
    if len(last_line) == 3:
        # Terminated at a constraint evaluation limit.
        assert lines[-1][1] == 'X'
        final = int(lines[-1][2])
    else:
        # Terminated at a quality limit (or BKDSAT proved exact?)
        assert len(last_line) == 2
        colours, checks = last_line
        improvements.append({'colours': int(colours), key: int(checks)})
        final = int(checks)

    return improvements, final


def run_lewis_algorithm(algorithm, instance_file, switches=None, options=None, effort_files=True):
    ''' Run one of the built executables on the given file, passing the
    given command line options. '''

    exec_path = pathlib.Path(__file__).parent / 'lewis' / algorithm / algorithm
    if not exec_path.exists():
        raise ValueError(f'Executable {exec_path.resolve()} was not found.')

    instance_path = pathlib.Path(instance_file)
    if not instance_path.exists():
        raise ValueError(f'Instance file {instance_path} was not found.')

    with temporary_directory() as path:

        # Run the algorithm from shell with a temporary directory for output files.
        command = [str(exec_path.resolve()), str(instance_path.resolve())]
        if switches:
            command.extend(['-' + switch for switch in switches])
        if options:
            for option, value in options.items():
                command.extend(['-' + option, str(value)])
        output = subprocess.check_output(command, cwd=path)

        # Parse the solution file, expecting a 0...N-1 colouring.
        solution_file = path / 'solution.txt'
        assert solution_file.exists()
        solution_values = [
            int(value) for value in
            solution_file.read_text().strip().split()]
        assert len(solution_values) == solution_values[0] + 1, 'Solution length mismatch'
        solution_values = solution_values[1:]
        ncolors = len(set(solution_values))
        assert set(solution_values) == set(range(ncolors)), 'Unexpected colour values'

        result = {'colours': ncolors, 'solution': solution_values}

        if effort_files:
            # Read effort txt files (for the complex algorithms).
            ceffort_file = path / 'ceffort.txt'
            assert ceffort_file.exists()
            ceffort_improvements, ceffort_final = read_effort_file(
                ceffort_file.read_text(), ncolors, 'constraint_checks')
            teffort_file = path / 'teffort.txt'
            assert teffort_file.exists()
            teffort_improvements, teffort_final = read_effort_file(
                teffort_file.read_text(), ncolors, 'time_ms')

            result.update({
                'constraint_effort': ceffort_improvements,
                'time_effort': teffort_improvements,
            })

        else:
            # Read effort from stdout.
            teffort_final, ceffort_final = parse_simple_stdout(output, ncolors)

    result.update({
        'constraint_checks': ceffort_final,
        'time_ms': teffort_final,
    })

    return result


def parse_simple_stdout(output, ncolors):
    lines = output.decode().strip().split('\n')
    assert tuple(lines[0].strip().split()) == ('COLS', 'CPU-TIME(ms)', 'CHECKS')
    cols, time_ms, checks = lines[1].strip().split()
    assert int(cols) == ncolors
    return int(time_ms.partition('ms')[0]), int(checks)


def common_options(max_checks=None, target_colours=None, random_seed=None):
    options = {}
    if max_checks is not None:
        options['s'] = max_checks
    if target_colours is not None:
        options['T'] = target_colours
    if random_seed is not None:
        options['r'] = random_seed
    return options


CONSTRUCTION_INDEX = {'dsatur': 1, 'greedy': 2}
CROSSOVER_INDEX = {'gpx': 1, 'gpx_kempe': 2, 'mpx': 3, 'gga': 4, 'npoint': 5}


def ant_colony(instance_file, *, max_checks=None, target_colours=None,
               random_seed=None, tabu_iterations=None):
    options = common_options(max_checks, target_colours, random_seed)
    if tabu_iterations is not None:
        options['I'] = tabu_iterations
    return run_lewis_algorithm('AntCol', instance_file, options=options)


def hill_climber(instance_file, *, max_checks=None, target_colours=None,
                 random_seed=None, local_iterations=None):
    options = common_options(max_checks, target_colours, random_seed)
    if local_iterations is not None:
        options['I'] = local_iterations
    return run_lewis_algorithm('HillClimber', instance_file, options=options)


def hybrid_ea(instance_file, *, max_checks=None, target_colours=None, random_seed=None,
              tabu_iterations=None, population_size=None, construction=None,
              crossover=None):
    options = common_options(max_checks, target_colours, random_seed)
    if tabu_iterations is not None:
        options['I'] = tabu_iterations
    if population_size is not None:
        options['p'] = population_size
    if construction is not None:
        options['a'] = CONSTRUCTION_INDEX[construction.lower()]
    if crossover is not None:
        options['x'] = CROSSOVER_INDEX[crossover.lower()]
    return run_lewis_algorithm('HybridEA', instance_file, options=options)


def partial_col(instance_file, *, max_checks=None, target_colours=None, random_seed=None,
                construction=None):
    options = common_options(max_checks, target_colours, random_seed)
    if construction is not None:
        options['a'] = CONSTRUCTION_INDEX[construction.lower()]
    return run_lewis_algorithm('PartialColAndTabuCol', instance_file, options=options)


def tabu_col(instance_file, *, max_checks=None, target_colours=None, random_seed=None,
             construction=None, dynamic_tenure=False):
    options = common_options(max_checks, target_colours, random_seed)
    if construction is not None:
        options['a'] = CONSTRUCTION_INDEX[construction.lower()]
    switches = ('t', 'tt') if dynamic_tenure else ('t',)
    return run_lewis_algorithm('PartialColAndTabuCol', instance_file, switches, options)


def backtracking_dsatur(instance_file, *, max_checks=None, target_colours=None, random_seed=None):
    options = common_options(max_checks, target_colours, random_seed)
    return run_lewis_algorithm('BacktrackingDSatur', instance_file, options=options)


def simple_greedy(instance_file, *, random_seed=None):
    options = common_options(random_seed=random_seed)
    return run_lewis_algorithm('SimpleGreedy', instance_file, ('v',), options, effort_files=False)


def dsatur(instance_file, *, random_seed=None):
    options = common_options(random_seed=random_seed)
    return run_lewis_algorithm('DSatur', instance_file, ('v',), options, effort_files=False)


if __name__ == '__main__':

    import itertools, json, tqdm, logging, multiprocessing
    from pprint import pprint

    def simple_result(algorithm, instance_file):
        try:
            result = algorithm(instance_file)
            return {
                key: value
                for key, value in result.items()
                if key in ['colours', 'constraint_checks', 'time_ms']
            }
        except Exception as e:
            logging.exception(f'Error {algorithm.__name__} on {instance_file.stem}', e)
            return None

    def evaluate(instance_file):
        return instance_file.stem, {
            algorithm.__name__: simple_result(algorithm, instance_file)
            for algorithm in [
                ant_colony, hill_climber, hybrid_ea, partial_col, tabu_col,
                backtracking_dsatur, simple_greedy, dsatur]}

    instance_files = sorted(pathlib.Path('graphs/instances').glob('g*.col'))
    pool = multiprocessing.Pool()
    results = list(tqdm.tqdm(pool.imap_unordered(evaluate, instance_files), total=len(instance_files)))

    print(f'Writing {len(results)} results.')
    with open('lewis-evolved-results.json', 'w') as outfile:
        json.dump(results, outfile, sort_keys=True, indent=4)
