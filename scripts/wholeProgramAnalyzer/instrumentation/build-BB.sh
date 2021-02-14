#!/bin/bash

# usage: ./build-BB.sh 50
# params: blackbox timeout (in minutes)

cpp_benchmarks=('lulesh' 'molecularDynamics' 'reactorSimulation')
c_benchmarks=('arclength' 'fbenchV1' 'fbenchV2' 'invertedPendulum' 'linearSVC' 'linpack' 'nbody' 'rayCasting')
# rust_benchmarks=('arclength' 'linearSVC' 'lulesh' 'invertedPendulum' 'rayCasting' 'nbody')

mkdir -p busy-files
mkdir -p build

runtime=$1
LLVM_DIR=$(find / -name "llvm-10")
BLOSSOM_DIR=$(find ../../../ -name "blossom")

BENCH_DIR=${BLOSSOM_DIR}/benchmarks
BB=${BLOSSOM_DIR}/src/black-box-pass/built/
# RUST_DIR=$(find ~/.rustup -wholename "*/lib/libstd*.so" -and -not -wholename "*rustlib*")

# Building and instrumenting cpp benchmarks

echo "**************** Building C++ benchmarks ****************"

for i in "${cpp_benchmarks[@]}"
do
  echo "Building benchmark:" ${i} ""
  seed=$RANDOM
  echo ${i} >> seed.txt
  echo ${seed} >> seed.txt
  $LLVM_DIR/build/bin/clang++ -emit-llvm -c $BENCH_DIR/c++/$i/$i.cc -o busy-files/$i.bc
  $LLVM_DIR/build/bin/opt -load $BB/black_box_pass.so -blackbox -time ${runtime} -seed ${seed} -lang c++ -config $BENCH_DIR/c++/$i/$i.config < busy-files/${i}.bc > busy-files/${i}_transform.bc 2>/dev/null
  $LLVM_DIR/build/bin/llc -filetype=obj busy-files/${i}_transform.bc
  $LLVM_DIR/build/bin/clang++ -O3 busy-files/${i}_transform.o -o build/${i}.out
done

# Building and instrumenting c benchmarks
echo "**************** Building C benchmarks ****************"

for i in "${c_benchmarks[@]}"
do
  echo "Building benchmark:" ${i} ""
  seed=$RANDOM
  echo ${i} >> seed.txt
  echo ${seed} >> seed.txt
  $LLVM_DIR/build/bin/clang -emit-llvm -c $BENCH_DIR/c/$i/$i.c -o busy-files/$i.bc
  $LLVM_DIR/build/bin/opt -load $BB/black_box_pass.so -blackbox -time ${runtime} -seed ${seed} -lang c -config $BENCH_DIR/c/$i/$i.config < busy-files/${i}.bc > busy-files/${i}_transform.bc 2>/dev/null
  $LLVM_DIR/build/bin/llc -filetype=obj busy-files/${i}_transform.bc
  $LLVM_DIR/build/bin/clang -O3 busy-files/${i}_transform.o -o build/${i}.out -lm
done

# Building and instrumenting rust benchmarks
# echo "**************** Building Rust benchmarks ****************"

# for i in "${rust_benchmarks[@]}"
# do
#   echo "Building benchmark:" ${i} ""
#   seed=$RANDOM
#   echo ${i} >> seed.txt
#   echo ${seed} >> seed.txt
#   rustc --crate-type lib -g --emit=llvm-bc $BENCH_DIR/rust/$i/$i.rs -o busy-files/$i.bc -A warnings
#   $LLVM_DIR/build/bin/opt -load $BB/black_box_pass.so -blackbox -time ${runtime} -seed ${seed} -lang rust -config $BENCH_DIR/rust/$i/$i.config < busy-files/${i}.bc > busy-files/${i}_transform.bc 2>/dev/null
#   $LLVM_DIR/build/bin/llc -filetype=obj busy-files/${i}_transform.bc
#   $LLVM_DIR/build/bin/clang++ -w -O3 -Xlinker $RUST_DIR busy-files/${i}_transform.o -o build/${i}_rust.out
# done

rm -r busy-files
