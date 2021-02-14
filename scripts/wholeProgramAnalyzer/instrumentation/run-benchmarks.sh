#!/bin/bash

# usage: ./run-benchmarks.sh <result_dir> <iteration>

benchmarks=('arclength' 'fbenchV1' 'fbenchV2' 'invertedPendulum' 'linearSVC' 'linpack' 'nbody'
 'rayCasting' 'lulesh' 'molecularDynamics' 'reactorSimulation'
#  'arclength_rust' 'linearSVC_rust' 'lulesh_rust' 'invertedPendulum_rust' 'rayCasting_rust' 'nbody_rust'
 )

RESULTS_DIRECTORY=$1
ITR=$2
COMBINATION=$3

mkdir -p results
echo "**************** Code instrumention done!! ***************************"
for i in "${benchmarks[@]}"
do
  echo "Running benchmark: "${i}""
  build/${i}.out > results/${i}.log
  mv results/${i}.log $RESULTS_DIRECTORY/$i/ex${ITR}/file.log
  if [ ${COMBINATION} == "bb+aflgo" ]
  then
    python ../resultGeneration/unifiedFormat.py -f $RESULTS_DIRECTORY/$i/ex${ITR}/file.log -e $i
  else
    python ../resultGeneration/unifiedFormat.py -f $RESULTS_DIRECTORY/$i/ex${ITR}/file.log -e $i --broad
  fi
  mv *kernel* $RESULTS_DIRECTORY/$i/ex${ITR}/
  rm $RESULTS_DIRECTORY/$i/ex${ITR}/file.log
done