# Blossom
Blossom[[1]](#1) fuzzes an input program to generate ranges of a set of identified numerical kernels in the program. It has two fuzzing techniques: 1) blackbox fuzzing that randomly generates program inputs and monitors the ranges of kernel inputs and 2) guided blackbox fuzzing that generates program inputs specifically with the objective to expand the ranges of kernel inputs.

We then use these ranges to verify numerical kernels with our extension of [Daisy](https://github.com/malyzajko/daisy/tree/cancellation). We provide the jar file of this extension here for easier use.

## Reference ##
<a id="1">[1]</a> 
[A Two-Phase Approach for Conditional Floating-Point Verification](https://people.mpi-sws.org/~dlohar/assets/documents/tacas2021.pdf), Debasmita Lohar, Clothilde Jeangoudoux, Joshua Sobel, Eva Darulova, and Maria Christakis, TACAS 2021.


## Software Dependencies ##
- CMAKE
- LLVM 10

(If Rust is used)
- Rust
- rustfilt

(If Daisy is used for kernel verification)
- libgmp10
- libmpfr4
- Z3

### Installation Instructions ###
CMAKE:
- ``` sudo apt-get install cmake ```

LLVM 10:
- ``` git clone https://github.com/llvm/llvm-project.git ```
- ``` cd llvm-project ```
- ``` git checkout release/10.x ```
- ``` mkdir build ```
- ``` cd build ```
- ``` cmake -DCMAKE_BUILD_TYPE=Release -DLLVM_TARGETS_TO_BUILD=X86 -DLLVM_ENABLE_PROJECTS=clang <llvm-project/root/dir>/llvm/ ```
- ``` cmake --build . ```

(If Rust is used)
Rust:
- ``` curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh ```
- ``` cargo install rustfilt ```


## Building Blossom ##
(from wholeProgramAnalyzer/blossom/src/black-box-pass / guided-black-box-pass directory)

- ``` cmake -DLT_LLVM_INSTALL_DIR=<llvm_install_directory>/llvm-project/build ```
- ``` make ```

## Running Blossom ##

**NOTE:** We provide all our benchmarks here: `wholeProgramAnalyzer/blossom/benchmarks/`. For running the scripts, we assume that you are in the main directory.

### Running Blossom on the a single benchmark program ###
- ``` bash scripts/wholeProgramAnalyzer/run-Blossom.sh <benchmark> <language> <time> <fuzz> <iteration> ```
- ``` python scripts/wholeProgramAnalyzer/resultGeneration/computeResults.py <result_dir> <table_num> ``` (produces results as shown in the paper)

e.g, running Blossom's BB on `arclength` for 50 minutes and 2 iterations:
- ``` bash scripts/wholeProgramAnalyzer/run-Blossom.sh arclength c 50 bb 2 ```
- ``` python scripts/wholeProgramAnalyzer/resultGeneration/computeResults.py results/blossom-bb 2 ``` (For Table 2 result)
- ``` python scripts/wholeProgramAnalyzer/resultGeneration/computeResults.py results/blossom-bb 3 ``` (For Table 3 result)

### Running Blossom on all benchmarks (as presented in the paper) ###
- ``` bash scripts/wholeProgramAnalyzer/Blossom-BB-experiments.sh 50 5 ``` (for 50 mins and 5 iterations)
- ``` bash scripts/wholeProgramAnalyzer/Blossom-GBB-experiments.sh 50 5 ``` (for 50 mins and 5 iterations)
- ``` python scripts/wholeProgramAnalyzer/resultGeneration/computeResults.py results/BB_50 2 ``` (result directory, table 2)
- ``` python scripts/wholeProgramAnalyzer/resultGeneration/computeResults.py results/guidedBB_50 2 ``` (result directory, table 2)
- ``` python scripts/wholeProgramAnalyzer/resultGeneration/computeResults.py results/BB_50 3 ``` (result directory, table 3)
- ``` python scripts/wholeProgramAnalyzer/resultGeneration/computeResults.py results/guidedBB_50 3 ``` (result directory, table 3)

### Kernel Analysis with Daisy of all numerical kernels (as presented in the paper) ###
- ``` bash scripts/kernelAnalyzer/kernelExperimentsDaisy.sh ``` (verifies all kernels)
- ``` bash scripts/kernelAnalyzer/optimizationDaisy.sh ``` (optimizes relevant kernels)

### Experimenting with new benchmarks ###
Blossom requires an input program file and a configuration file. The configuration file needs to have the same name as the program file with .config as the extension.

The configuration file lists the program input ranges.

Supported types: Numerical (int, float, double), Array, Pointer, Structure

#### Numerical type ####
- main\_**i**\_min = <lower\_limit> ( **i** indicates the argument number)
- main\_**i**\_min = <upper\_limit>

#### Arrays ####
- main\_i\_size = <array_len>
- main\_i\_**j\_k**\_min = <lower\_limit> (**j\_k** indicates **j_k**th element of the array, supports higher dimensions)
- main\_i\_**j\_k**\_max = <upper\_limit>

#### Pointers ####
- main\_i\_size = <ptr_len>
- main\_i\_min = <lower\_limit>
- main\_i\_max = <upper\_limit>

#### Kernel input array size ####
- numerical\_kernel**i**\_**j**\_size = <array\_len> (**j** indicates **j**th input of kernel **i** is an array of <array\_len>)

## Contributors ##
- Joshua Sobel
- Debasmita Lohar

## Acknowledgements ##
Some portions of the llvm infrustructure has been inspired by and sometimes directly taken from the example LLVM-passes of llvm-tutor project (see the LLVM_tutor_LICENSE).
