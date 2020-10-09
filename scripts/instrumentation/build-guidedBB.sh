#!/bin/bash

# usage: ./build-guidedBB.sh 50 5
# params: guided-blackbox timeout (in minutes), # mutations

rust_benchmarks=('arclength' 'linearSVC' 'lulesh' 'invertedPendulum' 'rayCasting' 'nbody')
cpp_benchmarks=('lulesh' 'molecularDynamics' 'reactorSimulation')
c_benchmarks=('arclength' 'fbenchV1' 'fbenchV2' 'invertedPendulum' 'linearSVC' 'linpack' 'nbody' 'rayCasting')

mkdir -p busy-files
mkdir -p build

runtime=$1
mutations=$2
RUST_DIR=$(find ~/.rustup -wholename "*/lib/libstd*.so" -and -not -wholename "*rustlib*")
LLVM_DIR=$(find / -name "llvm-project")

# Building and instrumenting cpp benchmarks
echo "**************** Building C++ benchmarks ****************"

for i in "${cpp_benchmarks[@]}"
do
  echo "Building benchmark:" ${i} ""
  seed=$RANDOM
  echo ${i} >> seed.txt
  echo ${seed} >> seed.txt
  $LLVM_DIR/build/bin/clang++ -emit-llvm -c ../../benchmarks/c++/$i/$i.cc -o busy-files/$i.bc
  $LLVM_DIR/build/bin/opt -load ../../src/guided-black-box-pass/built/guided_black_box_pass.so -guided-blackbox  -time ${runtime} -seed ${seed} -mutations ${mutations} -lang c++ -config ../../benchmarks/c++/$i/$i.config < busy-files/${i}.bc > busy-files/${i}_transform.bc 2>/dev/null
  $LLVM_DIR/build/bin/llc -filetype=obj busy-files/${i}_transform.bc
  $LLVM_DIR/build/bin/clang++ busy-files/${i}_transform.o -o build/${i}.out
done

# Building and instrumenting c benchmarks
echo "**************** Building C benchmarks ****************"

for i in "${c_benchmarks[@]}"
do
  echo "Building benchmark:" ${i} ""
  seed=$RANDOM
  echo ${i} >> seed.txt
  echo ${seed} >> seed.txt
  $LLVM_DIR/build/bin/clang -emit-llvm -c ../../benchmarks/c/$i/$i.c -o busy-files/$i.bc
  $LLVM_DIR/build/bin/opt -load ../../src/guided-black-box-pass/built/guided_black_box_pass.so -guided-blackbox -time ${runtime} -seed ${seed} -mutations ${mutations} -lang c -config ../../benchmarks/c/$i/$i.config < busy-files/${i}.bc > busy-files/${i}_transform.bc 2>/dev/null
  $LLVM_DIR/build/bin/llc -filetype=obj busy-files/${i}_transform.bc
  $LLVM_DIR/build/bin/clang busy-files/${i}_transform.o -o build/${i}.out -lm
done

# Building and instrumenting rust benchmarks
echo "**************** Building Rust benchmarks ****************"

for i in "${rust_benchmarks[@]}"
do
  echo "Building benchmark:" ${i} ""
  seed=$RANDOM
  echo ${i} >> seed.txt
  echo ${seed} >> seed.txt
  rustc --crate-type lib -g --emit=llvm-bc ../../benchmarks/rust/$i/$i.rs -o busy-files/$i.bc -A warnings
  $LLVM_DIR/build/bin/opt -load ../../src/guided-black-box-pass/built/guided_black_box_pass.so -guided-blackbox -time ${runtime} -seed ${seed} -mutations ${mutations} -lang rust -config ../../benchmarks/rust/$i/$i.config < busy-files/${i}.bc > busy-files/${i}_transform.bc 2>/dev/null
  $LLVM_DIR/build/bin/llc -filetype=obj busy-files/${i}_transform.bc
  $LLVM_DIR/build/bin/clang++ -Xlinker $RUST_DIR busy-files/${i}_transform.o -o build/${i}_rust.out
done

rm -r busy-files
