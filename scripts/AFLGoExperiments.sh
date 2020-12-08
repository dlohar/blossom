#!/bin/bash --posix

# usage: ./AFLGoExperiments.sh 5 45 5 
# params: AFLGo timeout (in minutes) to generate ranges, AFLGo timeout (in minutes) to broaden the ranges, iterations

# generate result directory
if [ ! -d ../results ]; then
  mkdir ../results
  mkdir ../results/aflgo
elif [ ! -d ../results/aflgo ]; then
  mkdir ../results/aflgo
fi

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

timeout=$1m+$2m

GR_DIR="../../generateRange"
BR_DIR="../../broadenRange"

RESULTS_DIRECTORY="../../../../results/aflgo"
		
cd ../comparison/aflgo/generateRange/
for ((i=1;i<=$3;i++)) # $3: iterations 
do
	for file in "${benchmarks[@]}"
  	do
		echo "****************************** Benchmark:" ${file} "******************************"
		echo "****************************** Generating initial ranges with AFLGo ******************************"
    	cd ${file}
    	./aflgo.sh $1 # $1: AFLGo timeout to generate ranges
    
    	# generate result directory
    	if [ ! -d $RESULTS_DIRECTORY/${file}${timeout} ]
    	then
      		mkdir $RESULTS_DIRECTORY/${file}${timeout}
    	fi
    	if [ ! -d $RESULTS_DIRECTORY/${file}${timeout}/ex$i ]
    	then
      		mkdir $RESULTS_DIRECTORY/${file}${timeout}/ex$i
    	fi

    	# save the results
    	cp build/kernel* $RESULTS_DIRECTORY/${file}${timeout}/ex$i/
    	cp build/in/in $RESULTS_DIRECTORY/${file}${timeout}/ex$i/in_GR.dat
  
		# Copy results for broaden_ranges
		mv build/kernel* $BR_DIR/${file}

    	# runs aflgo to broaden the initial ranges
    	echo "****************************** Broadening the range with AFLGo ******************************"
		cd $BR_DIR/${file}
		./aflgo.sh $2 # $2: AFLGo timeout to broaden the ranges
		cp build/broad_kernel* $RESULTS_DIRECTORY/${file}${timeout}/ex$i/
		cp build/in/in $RESULTS_DIRECTORY/${file}${timeout}/ex$i/in_BR.dat
	
		cd $GR_DIR
  	done
done
