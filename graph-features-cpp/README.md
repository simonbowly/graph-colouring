
# Graph feature calculation code

For the most part this is just a C++17 wrapper around the [igraph C library](https://igraph.org/c/).
The code is tested and built with `gcc` on ubuntu and requires the `igraph` library compiled and installed.

To build, run `make` from this directory.
The `bin/evaluate` executable takes DIMACS col format files on the command line and writes feature data to the console.
Run `make test` to check the basic code and `bin/evaluate test.col` to check reading DIMACS files.

The necessary functions can be called directly to construct graphs and calculate features.
Look at `test.cpp` for an example.
