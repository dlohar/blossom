#!/bin/bash --posix

# usage: ./guided-blackbox-fuzzer.sh 50 5 5
# params: guided blackbox timeout (in minutes), mutations, iterations

# generate result directory
if [ ! -d ../results ]; then
  mkdir ../results
  mkdir ../results/guidedBB_$1_$2
elif [ ! -d ../results/guidedBB_$1_$2 ]; then
  mkdir ../results/guidedBB_$1_$2
fi

RESULTS_DIRECTORY="../../results/guidedBB_$1_$2"
cd instrumentation
echo "******************** Range generation with Guided Black-box testing ********************"
# runs guided blackbox test to generate the ranges
for ((i=1;i<=$3;i++))
do
# generate result directory
  if [ ! -d $RESULTS_DIRECTORY/ex$i ]; then
    mkdir $RESULTS_DIRECTORY/ex$i
  fi
  ./build-guidedBB.sh $1 $2 # $1: timeout, $2: mutations
  ./run-benchmarks.sh
  mv results/* $RESULTS_DIRECTORY/ex$i
  mv seed.txt $RESULTS_DIRECTORY/ex$i
  rm -rf results
  rm -rf build
done

