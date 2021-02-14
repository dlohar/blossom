#!/bin/bash --posix

# usage: ./blackbox-fuzzer.sh 50 5
# params: blackbox timeout (in minutes), iterations
NEWPATH=$(realpath $0)
DIR=$(dirname $NEWPATH)
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
  mkdir ../../results/BB_$1
elif [ ! -d ../../results/BB_$1 ]; then
  mkdir ../../results/BB_$1
fi
for file in "${benchmarks[@]}"
do
  if [ ! -d ../../results/BB_$1/${file} ]
  then
    mkdir ../../results/BB_$1/${file}
    for ((i=1;i<=$2;i++))
    do
      mkdir ../../results/BB_$1/${file}/ex$i
    done
  else
    for ((i=1;i<=$2;i++))
    do
      if [ ! -d ../../results/BB_$1/${file}/ex$i ]
      then
        mkdir ../../results/BB_$1/${file}/ex$i
      fi
    done
  fi
done

cd instrumentation

echo "******************** Range generation with Black-box testing ********************"
# runs blackbox test to generate the ranges
for ((i=1;i<=$2;i++))
do
  ./build-BB.sh $1 #$1 : timeout
  ./run-benchmarks.sh ../../../results/BB_$1 $i bb
  mv seed.txt ../../../results/BB_$1/seed$i.txt
  rm -rf results
  rm -rf build
done


