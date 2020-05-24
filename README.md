# PathORAM

This is an implementation of the PathORAM algorithm from the [original paper](https://eprint.iacr.org/2013/280.pdf).
This implementation has the following features:
- it's written in C++ and is compilable into a standalone shared library (see [usage example](./path-oram/test/test-shared-lib.cpp))
- all components (storage, position map and stash) are abstracted via interfaces
- storage component can be
	- `InMemory` (using a preallocated heap array)
	- `FileSystem` (using a binary file)
	- `Redis` (using external Redis server and [C++ client](https://github.com/sewenew/redis-plus-plus), supports batch read/write)
	- `Aerospike` (using external Aerospike server and [official C client](https://www.aerospike.com/docs/client/c/), supports batch read, no batch write)
- solution can optionally be compiled without support for some storage adapters (`InMemory` and `FilesSystem` are always included)
- position map can be either in-memory, or using another PathORAM, thus enabling arbitrary-level recursive PathORAM
- an optimization for multiple requests at a time (mixed get and put)
- PRG and encryption are done with OpenSSL, encryption is AES-CBC-256 (or AES-CTR-256), random IV every time
- the solution is tested, the coverage is 100%
- the solution is benchmarked
- the solution is documented, the documentation is [online](https://pathoram.dbogatov.org/)
- user inputs are screened (exceptions are thrown if the input is invalid)
- Makefile is sophisticated - with simple commands one can compile and run tests and benchmarks
- the code is formatted according to [clang file](./.clang-format)
- there are VS Code configs that let one debug the code with breakpoints

## How to compile and run

Dependencies:
- for building a shared library
	- `g++` that supports `--std=c++17`
	- `make`
	- these libs `-l boost_system -l ssl -l crypto`
		- if `Redis` storage adapter is needed, also `-l redis++ -l hiredis`
		- if `Aerospike` storage adapter is needed, also `-l aerospike -l dl -l z`
- following configurations are supported:
	- `make ... CPPFLAGS="-DINPUT_CHECK=false` will skip "inputs checks" (such as that AES key is `KEYSIZE`)
	- `make ... CPPFLAGS="-DUSE_REDIS=false` will not compile Redis storage adapter (thus, dependencies not needed)
	- `make ... CPPFLAGS="-DUSE_AEROSPIKE=false` will not compile Aerospike storage adapter (thus, dependencies not needed)
	- you can combine the options `make ... CPPFLAGS="-DUSE_AEROSPIKE=false -DUSE_REDIS=false -DINPUT_CHECKS=false"`
- for testing and benchmarking
	- all of the above
	- these libs `-l gtest -l pthread -l benchmark -l gmock`
	- `gcovr` for coverage
	- `doxygen` for generating docs
- or, use this Docker image: `dbogatov/docker-images:pbc-latest`
- note, to test against storage adapters other than `InMemory` and `FileSystem`
	- To use `Redis`, a Redis instance has to be accessible over `tcp://127.0.0.1:6379`
		- For example: `docker run -it -p 6379:6379 redis`
	- To use `Aerospike`, an Aerospike instance has to be accessible over `127.0.0.1:3000`
		- For example: `docker run -it -p 3000:3000 aerospike/aerospike-server`

[Makefile](./path-oram/Makefile) is used for everything.

```bash
# to only compile shared library
make clean shared

# to only compile shared library without Redis and Aerospike
make clean shared CPPFLAGS="-DUSE_AEROSPIKE=false -DUSE_REDIS=false"

# to compile the shared library (libpathoram.so in ./bin) and run example code against it
make clean run-shared

# to compile and run all unit tests
make clean run-tests

# to compile and run all integration tests (usually heavier than units)
make clean run-integration

# to compile and run all benchmarks
make clean run-benchmarks

# to compute unit test coverage
make clean coverage

# to generate documentation
make clean docs
```
