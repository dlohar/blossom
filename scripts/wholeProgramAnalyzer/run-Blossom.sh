#!/bin/bash

# usage:
# blackbox: ./run-Blossom.sh arclength c 50 bb 5
# params: benchmark, language, timeout (in minutes), fuzzing, iterations
# guided-blackbox: ./blossom.sh test.cc/test.c c++/c 50 gbb 5
# params: benchmark, language, timeout (in minutes), fuzzing, iterations
NEWPATH=$(realpath $0)
DIR=$(dirname $NEWPATH)
cd $DIR

benchmark=$1
language=$2
runtime=$3
fuzz=$4
mutatant=5

LLVM_DIR=$(find / -name "llvm-10")
BLOSSOM_DIR=$(find ../../../ -name "blossom")

BENCH_DIR=${BLOSSOM_DIR}/benchmarks
BB=${BLOSSOM_DIR}/executables/black_box_pass.so
GBB=${BLOSSOM_DIR}/executables/guided_black_box_pass.so

# generate result directory
if [ ! -d ../../results ]; then
  mkdir ../../results
  mkdir ../../results/blossom-${fuzz}
  mkdir ../../results/blossom-${fuzz}/${benchmark}
elif [ ! -d ../../results/blossom-${fuzz} ]; then
  mkdir ../../results/blossom-${fuzz}
  mkdir ../../results/blossom-${fuzz}/${benchmark}
elif [ ! -d ../../results/blossom-${fuzz}/${benchmark} ]; then
  mkdir ../../results/blossom-${fuzz}/${benchmark}
fi

for ((i=1;i<=$5;i++)) # $5: iterations
do
  mkdir -p busy-files
  echo "**************** Building benchmark: ${benchmark} ****************"
  seed=$RANDOM
  echo ${benchmark} >> seed.txt
  echo ${seed} >> seed.txt

  if [ ${language} == "c" ]
  then
    $LLVM_DIR/build/bin/clang -emit-llvm -c ${BENCH_DIR}/${language}/${benchmark}/*.c -o busy-files/test.bc
  elif [ ${language} == "c++" ]
  then
    $LLVM_DIR/build/bin/clang++ -emit-llvm -c ${BENCH_DIR}/${language}/${benchmark}/*.cc -o busy-files/test.bc
  else
    rustc --crate-type lib -g --emit=llvm-bc ${BENCH_DIR}/${language}/${benchmark}/*.rs -o busy-files/test.bc -A warnings
  fi

  if [ ${fuzz} == "bb" ]
  then
    $LLVM_DIR/build/bin/opt -load $BB -blackbox -time ${runtime} -seed ${seed} -lang ${language} -config ${BENCH_DIR}/${language}/${benchmark}/*.config < busy-files/test.bc > busy-files/test_transform.bc 2>/dev/null
  else
    $LLVM_DIR/build/bin/opt -load $GBB -guided-blackbox  -time ${runtime} -seed ${seed} -mutations ${mutatant} -lang ${language} -config ${BENCH_DIR}/${language}/${benchmark}/*.config < busy-files/test.bc > busy-files/test_transform.bc 2>/dev/null
  fi

  $LLVM_DIR/build/bin/llc -filetype=obj busy-files/test_transform.bc

  if [ ${language} == "c" ]
  then
    $LLVM_DIR/build/bin/clang -O3 busy-files/test_transform.o -o test.out -lm
  elif [ ${language} == "c++" ]
  then
    $LLVM_DIR/build/bin/clang++ -O3 busy-files/test_transform.o -o test.out
  else
    RUST_DIR=$(find ~/.rustup -wholename "*/lib/libstd*.so" -and -not -wholename "*rustlib*")
    $LLVM_DIR/build/bin/clang++ -w -O3 -Xlinker $RUST_DIR busy-files/test_transform.o -o test.out
  fi

  rm -r busy-files

  echo "**************** Code instrumention done!! ***************************"

  echo "Running benchmark: "${benchmark}""
  ./test.out > file.log
  rm test.out
  python resultGeneration/unifiedFormat.py -f file.log -e ${benchmark} --broad
  rm file.log
  # generate result directory
  if [ ! -d ../../results/blossom-${fuzz}/${benchmark}/ex$i ]
  then
    mkdir ../../results/blossom-${fuzz}/${benchmark}/ex$i
  fi
  mv broad_kernel* ../../results/blossom-${fuzz}/${benchmark}/ex$i/
done
