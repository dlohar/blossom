# Blossom
Blossom fuzzes an input program to generate ranges of the identified numerical kernels in the program. It has two fuzzing techniques: 1) blackbox fuzzing that randomly generates program inputs and monitors the ranges of kernel inputs and 2) guided blackbox fuzzing that generates program inputs specifically with the objective to expand the ranges of kernel inputs.


## Software Dependencies ##
- CMAKE
- LLVM 10

(If Rust is used)
- Rust
- rustfilt


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
(from black-box-pass / guided-black-box-pass directory)

- ``` cmake -DLT_LLVM_INSTALL_DIR=<llvm_install_directory>/llvm-project/build ```
- ``` make ```


## Running Blossom ##

It requires an input program file and a configuration file. The configuration file needs to have the same name as the program file with .config as the extension.

### Configuration File Instructions ###
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

### Running Blossom on the test program ###
- ``` ./blossom.sh test.c c <time> bb (for blackbox)```
- ``` ./blossom.sh test.c c <time> guided-bb <mutants> (for guided- blackbox)```

### Running all benchmarks ###
(from the scripts directory)

- ``` ./blackboxFuzzer.sh <time> <iteration> ```
- ``` ./guidedBlackboxFuzzer.sh <time> <mutations> <iterations> ```


## Contributors ##
- Joshua Sobel
- Debasmita Lohar

## Acknowledgements ##
Some portions of the llvm infrustructure has been inspired by and sometimes directly taken from the example LLVM-passes of llvm-tutor project (see the LLVM_tutor_LICENSE).
