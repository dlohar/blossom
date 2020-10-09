#!/bin/bash

# usage:
# blackbox: ./blossom.sh test.cc/test.c/test.rs c++/c/rust 50 bb
# params: benchmark, language, timeout (in minutes), fuzzing
# guided-blackbox: ./blossom.sh test.cc/test.c/test.rs c++/c/rust 50 guided-bb 5
# params: benchmark, language, timeout (in minutes), fuzzing, no. of mutants

benchmark=$1
filename=${benchmark%%.*}
language=$2
runtime=$3
fuzz=$4
mutatant=$5

if [ ${fuzz} == "guided-bb" ]
then
  if [ $# -eq 4 ]; then
    echo "run guided-bb with no. of mutatants as the last parameter"
    exit 1
  fi
fi

mkdir -p busy-files

LLVM_DIR=$(find / -name "llvm-project")

echo "**************** Building benchmark: ${filename} ****************"
seed=$RANDOM
echo ${benchmark} >> seed.txt
echo ${seed} >> seed.txt

if [ ${lang} != "rust" ]
then
  $LLVM_DIR/build/bin/clang++ -emit-llvm -c ${benchmark} -o busy-files/${filename}.bc
else
  rustc --crate-type lib -g --emit=llvm-bc ${benchmark} -o busy-files/${filename}.bc -A warnings
fi

if [ ${fuzz} == "bb" ]
then
  $LLVM_DIR/build/bin/opt -load src/black-box-pass/built/black_box_pass.so -blackbox -time ${runtime} -seed ${seed} -lang ${language} -config ${filename}.config < busy-files/${filename}.bc > busy-files/${filename}_transform.bc 2>/dev/null
else
  $LLVM_DIR/build/bin/opt -load src/guided-black-box-pass/built/guided_black_box_pass.so -guided-blackbox  -time ${runtime} -seed ${seed} -mutations ${mutatant} -lang ${language} -config ${filename}.config < busy-files/${filename}.bc > busy-files/${filename}_transform.bc 2>/dev/null
fi

$LLVM_DIR/build/bin/llc -filetype=obj busy-files/${filename}_transform.bc

if [ ${lang} == "c" ]
then
  $LLVM_DIR/build/bin/clang -O3 busy-files/${filename}_transform.o -o ${filename}.out -lm
elif [ ${lang} == "c++" ]
then
  $LLVM_DIR/build/bin/clang++ -O3 busy-files/${benchmark}_transform.o -o ${filename}.out
else
  RUST_DIR=$(find ~/.rustup -wholename "*/lib/libstd*.so" -and -not -wholename "*rustlib*")
  $LLVM_DIR/build/bin/clang++ -w -O3 -Xlinker $RUST_DIR busy-files/${filename}_transform.o -o ${filename}.out
fi

rm -r busy-files

echo "**************** Code instrumention done!! ***************************"

echo "Running benchmark: "${filename}""
${filename}.out > results/${filename}.log
