#!/bin/bash --posix

# usage: ./guided-blackbox-fuzzer.sh 50 5
# params: guided blackbox timeout (in minutes), iterations
NEWPATH=$(realpath $0)
DIR=$(dirname $NEWPATH)
MUTATIONS=5
cd $DIR

declare -a benchmarks=(
  "arclength" \
  "fbenchV1" \
  "fbenchV2" \
  "invertedPendulum" \
  "linearSVC" \
  "linpack" \
  "nbody" \
  "rayCasting" \
  "lulesh" \
  "molecularDynamics" \
  "reactorSimulation"
)

# generate result directories
if [ ! -d ../../results ]; then
  mkdir ../../results
  mkdir ../../results/guidedBB_$1
elif [ ! -d ../../results/guidedBB_$1 ]; then
  mkdir ../../results/guidedBB_$1
fi
for file in "${benchmarks[@]}"
do
  if [ ! -d ../../results/guidedBB_$1/${file} ]
  then
    mkdir ../../results/guidedBB_$1/${file}
    for ((i=1;i<=$2;i++))
    do
      mkdir ../../results/guidedBB_$1/${file}/ex$i
    done
  else
    for ((i=1;i<=$2;i++))
    do
      if [ ! -d ../../results/guidedBB_$1/${file}/ex$i ]
      then
        mkdir ../../results/guidedBB_$1/${file}/ex$i
      fi
    done
  fi
done

cd instrumentation
echo "******************** Range generation with Guided Black-box testing ********************"
# runs guided blackbox test to generate the ranges
for ((i=1;i<=$2;i++))
do
  ./build-guidedBB.sh $1 $MUTATIONS # $1: timeout
  ./run-benchmarks.sh ../../../results/guidedBB_$1 $i gbb
  mv seed.txt ../../../results/guidedBB_$1/seed$i.txt
  rm -rf results
  rm -rf build
done
