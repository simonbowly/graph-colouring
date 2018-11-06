
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


def run_lewis_algorithm(algorithm, instance_file):

    exec_path = pathlib.Path(__file__).parent / 'lewis' / algorithm / algorithm
    if not exec_path.exists():
        raise ValueError(f'Executable {exec_path.resolve()} was not found.')

    instance_path = pathlib.Path(instance_file)
    if not instance_path.exists():
        raise ValueError(f'Instance file {instance_path} was not found.')

    with temporary_directory() as path:

        # Run the algorithm from shell with a temporary directory for output files.
        output = subprocess.check_output([
            str(exec_path.resolve()),
            str(instance_path.resolve())
            ], cwd=path)
        solution_file = path / 'solution.txt'
        assert solution_file.exists()

        # ceffort_file = path / 'ceffort.txt'
        # assert ceffort_file.exists()
        # teffort_file = path / 'teffort.txt'
        # assert teffort_file.exists()

        # Parse the solution file, expecting a 0...N-1 colouring.
        solution_values = [
            int(value) for value in
            solution_file.read_text().strip().split()]
        assert len(solution_values) == solution_values[0] + 1, 'Solution length mismatch'
        solution_values = solution_values[1:]
        ncolors = len(set(solution_values))
        assert set(solution_values) == set(range(ncolors)), 'Unexpected colour values'

    return {
        'ncolors': ncolors,
        'solution': solution_values
    }


if __name__ == '__main__':

    import itertools, json, tqdm, logging

    def run_algorithm(algorithm, instance_file):
        try:
            result = run_lewis_algorithm(algorithm, instance_file)
            return result['ncolors']
        except KeyboardInterrupt as e:
            raise
        except Exception as e:
            logging.exception(f'{algorithm} failed on {instance_file}', e)
            return None

    def evaluate(instance_file):
        return {
            algorithm: run_algorithm(algorithm, instance_file)
            for algorithm in [
                'AntCol', 'BacktrackingDSatur', 'DSatur', 'HillClimber',
                'HybridEA', 'PartialColAndTabuCol', 'RLF', 'SimpleGreedy']}

    instance_files = sorted(pathlib.Path('graphs/instances').glob('*.col'))
    results = {}
    try:
        for instance_file in tqdm.tqdm(instance_files):
            results[instance_file.name] = evaluate(instance_file)
    except KeyboardInterrupt:
        print('Interrupted')

    print(f'Writing {len(results)} results.')
    with open('results.json', 'w') as outfile:
        json.dump(results, outfile, sort_keys=True, indent=4)
