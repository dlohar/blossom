#!/bin/bash --posix
NEWPATH=$(realpath $0)
DIR=$(dirname $NEWPATH)
cd $DIR

echo "Verifying arclength kernel:"
./daisy --silent ../../kernelAnalyzer/kernels/daisy/Arclength.scala

echo "Verifying LinearSVC kernel:"
./daisy --silent ../../kernelAnalyzer/kernels/daisy/LinearSVC.scala

echo "Verifying Raycasting kernel:"
./daisy --silent ../../kernelAnalyzer/kernels/daisy/RayCasting.scala

echo "Verifying Nbody kernel1:"
./daisy --silent ../../kernelAnalyzer/kernels/daisy/NbodyKernel1.scala
echo "Verifying Nbody kernel2:"
./daisy --silent ../../kernelAnalyzer/kernels/daisy/NbodyKernel2.scala

echo "Verifying InvertedPendulum kernel 1:"
./daisy --silent ../../kernelAnalyzer/kernels/daisy/InvertedPendulumKernel1.scala
echo "Verifying InvertedPendulum kernel 2:"
./daisy --silent ../../kernelAnalyzer/kernels/daisy/InvertedPendulumKernel2.scala

echo "Verifying FBenchV2 (with library function) kernel 1:"
./daisy ../../kernelAnalyzer/kernels/daisy/FbenchV2Kernel1.scala
echo "Verifying FBenchV2 (with library function) kernel 2:"
./daisy ../../kernelAnalyzer/kernels/daisy/FbenchV2Kernel2.scala

echo "Verifying FBenchV1 (without library function) kernel 1:"
./daisy --silent ../../kernelAnalyzer/kernels/daisy/FbenchV1Kernel1.scala
echo "Verifying FBenchV1 (without library function) kernel 2:"
./daisy --silent ../../kernelAnalyzer/kernels/daisy/FbenchV1Kernel2.scala
echo "Verifying FBenchV1 (without library function) kernel 3:"
./daisy --silent ../../kernelAnalyzer/kernels/daisy/FbenchV1Kernel3.scala
echo "Verifying FBenchV1 (without library function) kernel 4:"
./daisy --silent ../../kernelAnalyzer/kernels/daisy/FbenchV1Kernel4.scala

echo "Verifying Linpack kernels:"
./daisy --silent ../../kernelAnalyzer/kernels/daisy/Linpack.scala

echo "Verifying MolecularDynamics kernel 1:"
./daisy --silent ../../kernelAnalyzer/kernels/daisy/MolecularDynamicsKernel1.scala
echo "Verifying MolecularDynamics kernel 2:"
./daisy --silent ../../kernelAnalyzer/kernels/daisy/MolecularDynamicsKernel2.scala
echo "Verifying MolecularDynamics kernel 3:"
./daisy --silent ../../kernelAnalyzer/kernels/daisy/MolecularDynamicsKernel3.scala

echo "Verifying ReactorSimulation kernel 1:"
./daisy --silent ../../kernelAnalyzer/kernels/daisy/ReactorSimulationKernel1.scala
echo "Verifying ReactorSimulation kernel 2:"
./daisy --silent ../../kernelAnalyzer/kernels/daisy/ReactorSimulationKernel2.scala
echo "Verifying ReactorSimulation kernel 3:"
./daisy --silent ../../kernelAnalyzer/kernels/daisy/ReactorSimulationKernel3.scala

echo "Verifying Lulesh kernel 1:"
./daisy --silent ../../kernelAnalyzer/kernels/daisy/LuleshKernel1.scala
echo "Verifying Lulesh kernel 2:"
./daisy --silent ../../kernelAnalyzer/kernels/daisy/LuleshKernel2.scala
echo "Verifying Lulesh kernel 3:"
./daisy --silent ../../kernelAnalyzer/kernels/daisy/LuleshKernel3.scala
echo "Verifying Lulesh kernel 4:"
./daisy --silent ../../kernelAnalyzer/kernels/daisy/LuleshKernel4.scala
