#! /bin/bash
# AFLGO execution script for inverted_pendulum_aflgo.c benchmark

# Clean before execution
./clean.sh

# Setup directory containing all temporary files
mkdir obj-aflgo; mkdir obj-aflgo/temp

#finds the source of aflgo"
SRCDIR=$(find / ! -path '/proc' -name "aflgo-src")

# Set aflgo-instrumenter and flags
export AFLGO="$SRCDIR"
export SUBJECT=$PWD; export TMP_DIR=$PWD/obj-aflgo/temp
export CC=$AFLGO/afl-clang-fast; export CXX=$AFLGO/afl-clang-fast++
export LDFLAGS=-lpthread
export ADDITIONAL="-targets=$TMP_DIR/BBtargets.txt -outdir=$TMP_DIR -flto -fuse-ld=gold -Wl,-plugin-opt=save-temps"

# AFLGO Target
# ------------ Kernel-file:Kernel-line -----------------------------------
echo $'invertedPendulum_aflgo.c:77\ninvertedPendulum_aflgo.c:96' > $TMP_DIR/BBtargets.txt

# Build with generic build.sh file 
# ------------ modify CMakeLists.txt according to source name ------------
LINK=STATIC CFLAGS="$ADDITIONAL" CXXFLAGS="$ADDITIONAL" ./build.sh

# Clean up
cat $TMP_DIR/BBnames.txt | rev | cut -d: -f2- | rev | sort | uniq > $TMP_DIR/BBnames2.txt && mv $TMP_DIR/BBnames2.txt $TMP_DIR/BBnames.txt
cat $TMP_DIR/BBcalls.txt | sort | uniq > $TMP_DIR/BBcalls2.txt && mv $TMP_DIR/BBcalls2.txt $TMP_DIR/BBcalls.txt

# Generate distance 
# ------------ Name of the executable ------------------------------------
cd build/; $AFLGO/scripts/genDistance.sh $SUBJECT $TMP_DIR inverted_pendulum_aflgo
cd -; rm -rf build; LINK=STATIC CFLAGS="-g -distance=$TMP_DIR/distance.cfg.txt" CXXFLAGS="-g -distance=$TMP_DIR/distance.cfg.txt" ./build.sh

# Check distance file
echo "Distance values:"
head -n5 $TMP_DIR/distance.cfg.txt
echo "..."
tail -n5 $TMP_DIR/distance.cfg.txt

# Fuzz with AFLGO
cd build/; mkdir in
# ------------ Input to fuzz here ----------------------------------------
# echo "0.0
# 0.0
# 3.0
# 0.0" > in/in
IN1=`./../../../getInput/get_rand_float.sh 0.0 0.0009`
IN2=`./../../../getInput/get_rand_float.sh 0.0 0.0009`
IN3=`./../../../getInput/get_rand_float.sh 1.5707963 3.14159265`
IN4=`./../../../getInput/get_rand_float.sh 0.0 0.0009`
echo "$IN1 $IN2 $IN3 $IN4" > in/in
touch kernel1_range.dat
touch kernel2_range.dat
# ------------ Name of the executable ------------------------------------
# ----- Set timeout -> in secs (s), mins (m), hrs (h), or days (d) -------
timeout "$1"m $AFLGO/afl-fuzz -m none -z exp -c 45m -i in -o out ./invertedPendulum_aflgo

