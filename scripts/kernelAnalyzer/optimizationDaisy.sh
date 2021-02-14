#!/bin/bash --posix
NEWPATH=$(realpath $0)
DIR=$(dirname $NEWPATH)
cd $DIR

echo "Optimizing LinearSVC kernel:"
./daisy --silent --rewrite ../../kernelAnalyzer/kernels/daisy/LinearSVC.scala

echo "Optimizing InvertedPendulum kernel 2:"
./daisy --silent --rewrite ../../kernelAnalyzer/kernels/daisy/InvertedPendulumKernel2.scala

echo "Optimizing Lulesh kernel 1:"
./daisy --silent --rewrite ../../kernelAnalyzer/kernels/daisy/LuleshKernel1.scala
echo "Optimizing Lulesh kernel 2:"
./daisy --silent --rewrite ../../kernelAnalyzer/kernels/daisy/LuleshKernel2.scala
echo "Optimizing Lulesh kernel 3:"
./daisy --silent --rewrite ../../kernelAnalyzer/kernels/daisy/LuleshKernel3.scala
echo "Optimizing Lulesh kernel 4:"
./daisy --silent --rewrite ../../kernelAnalyzer/kernels/daisy/LuleshKernel4.scala

echo "Optimizing MolecularDynamics kernel 2:"
./daisy --silent --rewrite ../../kernelAnalyzer/kernels/daisy/MolecularDynamicsKernel2.scala
echo "Optimizing MolecularDynamics kernel 3:"
./daisy --silent --rewrite ../../kernelAnalyzer/kernels/daisy/MolecularDynamicsKernel3.scala
