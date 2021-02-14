#! /bin/bash
# AFLGO execution script for nbody_aflgo.c benchmark

# Clean before execution
./clean.sh

# Setup directory containing all temporary files
mkdir obj-aflgo; mkdir obj-aflgo/temp

#finds the source of aflgo"
SRCDIR=$(find ../../../ -name "aflgo")

# Set aflgo-instrumenter and flags
export AFLGO="$SRCDIR"
export SUBJECT=$PWD; export TMP_DIR=$PWD/obj-aflgo/temp
export CC=$AFLGO/afl-clang-fast; export CXX=$AFLGO/afl-clang-fast++
export LDFLAGS=-lpthread
export ADDITIONAL="-targets=$TMP_DIR/BBtargets.txt -outdir=$TMP_DIR -flto -fuse-ld=gold -Wl,-plugin-opt=save-temps"

# AFLGO Target
# ------------ Kernel-file:Kernel-line -----------------------------------
echo $'nbody_aflgo.c:66\nnbody_aflgo.c:70\nnbody_aflgo.c:102\nnbody_aflgo.c:106' > $TMP_DIR/BBtargets.txt

# Build with generic build.sh file 
# ------------ modify CMakeLists.txt according to source name ------------
LINK=STATIC CFLAGS="$ADDITIONAL" CXXFLAGS="$ADDITIONAL" ./build.sh

# Clean up
cat $TMP_DIR/BBnames.txt | rev | cut -d: -f2- | rev | sort | uniq > $TMP_DIR/BBnames2.txt && mv $TMP_DIR/BBnames2.txt $TMP_DIR/BBnames.txt
cat $TMP_DIR/BBcalls.txt | sort | uniq > $TMP_DIR/BBcalls2.txt && mv $TMP_DIR/BBcalls2.txt $TMP_DIR/BBcalls.txt

# Generate distance 
# ------------ Name of the executable ------------------------------------
cd build/; $AFLGO/scripts/genDistance.sh $SUBJECT $TMP_DIR nbody_aflgo
cd -; rm -rf build; LINK=STATIC CFLAGS="-g -distance=$TMP_DIR/distance.cfg.txt" CXXFLAGS="-g -distance=$TMP_DIR/distance.cfg.txt" ./build.sh

# Check distance file
echo "Distance values:"
head -n5 $TMP_DIR/distance.cfg.txt
echo "..."
tail -n5 $TMP_DIR/distance.cfg.txt

# Fuzz with AFLGO
cd build/; mkdir in
# ------------ Input to fuzz here ----------------------------------------
echo "1.0 0.0 0.0 0.0 0.01 0.0 0.0
0.1 1.0 1.0 0.0 0.0 0.0 0.02
0.001 0.0 1.0 1.0 0.01 -0.01 -0.01" > in/in
# ------------ Name of the executable ------------------------------------
# ----- Set timeout -> in secs (s), mins (m), hrs (h), or days (d) -------
timeout "$1"m $AFLGO/afl-fuzz -m none -z exp -c 45m -i in -o out ./nbody_aflgo

