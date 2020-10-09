#!/bin/bash --posix

# usage: ./blackbox-fuzzer.sh 50 5
# params: blackbox timeout (in minutes), iterations

# generate result directory
if [ ! -d ../results ]; then
  mkdir ../results
  mkdir ../results/BB_$1
elif [ ! -d ../results/BB_$1 ]; then
  mkdir ../results/BB_$1
fi

RESULTS_DIRECTORY="../../results/BB_$1"
cd instrumentation
echo "******************** Range generation with Black-box testing ********************"
# runs blackbox test to generate the ranges
for ((i=1;i<=$2;i++))
do
# generate result directory
  if [ ! -d $RESULTS_DIRECTORY/ex$i ]; then
    mkdir $RESULTS_DIRECTORY/ex$i
  fi
  ./build-BB.sh $1 #$1 : timeout
  ./run-benchmarks.sh
  mv results/* $RESULTS_DIRECTORY/ex$i
  mv seed.txt $RESULTS_DIRECTORY/ex$i
  rm -rf results
  rm -rf build
done

