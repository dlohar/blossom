#! /bin/bash
# AFLGO execution script for arclength_aflgo.c benchmark

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
echo $'compute_range.c:24\ncompute_range.c:26' > $TMP_DIR/BBtargets.txt

# Build with generic build.sh file 
# ------------ modify CMakeLists.txt according to source name ------------
LINK=STATIC CFLAGS="$ADDITIONAL" CXXFLAGS="$ADDITIONAL" ./build.sh

# Clean up
cat $TMP_DIR/BBnames.txt | rev | cut -d: -f2- | rev | sort | uniq > $TMP_DIR/BBnames2.txt && mv $TMP_DIR/BBnames2.txt $TMP_DIR/BBnames.txt
cat $TMP_DIR/BBcalls.txt | sort | uniq > $TMP_DIR/BBcalls2.txt && mv $TMP_DIR/BBcalls2.txt $TMP_DIR/BBcalls.txt

# Generate distance 
# ------------ Name of the executable ------------------------------------
cd build/; $AFLGO/scripts/genDistance.sh $SUBJECT $TMP_DIR fbench_aflgo
cd -; rm -rf build; LINK=STATIC CFLAGS="-g -distance=$TMP_DIR/distance.cfg.txt" CXXFLAGS="-g -distance=$TMP_DIR/distance.cfg.txt" ./build.sh

# Check distance file
echo "Distance values:"
head -n5 $TMP_DIR/distance.cfg.txt
echo "..."
tail -n5 $TMP_DIR/distance.cfg.txt

# Fuzz with AFLGO
cd build/; mkdir in
# ------------ Input to fuzz here ----------------------------------------
# echo "7600.0 6800.955 6500.816 5800.944 5200.557 4800.344 4300.477 3900.494" > in/in
IN1=`./../../../getInput/get_rand_float.sh 7521.0 7621.0`
IN2=`./../../../getInput/get_rand_float.sh 6769.955 6869.955`
IN3=`./../../../getInput/get_rand_float.sh 6462.816 6562.816`
IN4=`./../../../getInput/get_rand_float.sh 5795.944 5895.944`
IN5=`./../../../getInput/get_rand_float.sh 5169.557 5269.557`
IN6=`./../../../getInput/get_rand_float.sh 4761.344 4861.344`
IN7=`./../../../getInput/get_rand_float.sh 4240.477 4340.477`
IN8=`./../../../getInput/get_rand_float.sh 3868.494 3968.494`
echo "$IN1 $IN2 $IN3 $IN4 $IN5 $IN6 $IN7 $IN8" > in/in
touch broad_kernel1_range.dat
touch broad_kernel2_range.dat
touch broad_kernel3_range.dat
touch broad_kernel4_range.dat
touch broad_kernel5_range.dat
# ------------ Name of the executable ------------------------------------
# ----- Set timeout -> in secs (s), mins (m), hrs (h), or days (d) -------
timeout "$1"m $AFLGO/afl-fuzz -m none -z exp -c 45m -i in -o out ./fbench_aflgo

