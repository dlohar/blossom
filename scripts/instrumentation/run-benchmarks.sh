#!/bin/bash

# usage: ./run-benchmarks.sh

benchmarks=('arclength' 'fbenchV1' 'fbenchV2' 'invertedPendulum' 'linearSVC' 'linpack' 'nbody'
 'rayCasting'
 'lulesh' 'molecularDynamics' 'reactorSimulation'
 'arclength_rust' 'linearSVC_rust' 'lulesh_rust' 'invertedPendulum_rust' 'rayCasting_rust' 'nbody_rust')

mkdir -p results
echo "**************** Code instrumention done!! ***************************"
for i in "${benchmarks[@]}"
do
  echo "Running benchmark: "${i}""
  build/${i}.out > results/${i}.log
done

