#!/bin/bash

outpath="/tmp/scalability-test/"
coreCloudNodeVector=(1 4 8 12 16 20 24 28 32)
#coreCloudNodeVector=(1 4 8)

################## STEP [0] ##################
echo "===================== FISIM Scalability TEST =====================\n"
echo ""
cd ../

mkdir -p $outpath
./ns3 build

for coreCloudNode in ${coreCloudNodeVector[@]}; do
    echo "coreCloudNode: $coreCloudNode"
    # Run the simulation
    ./ns3 run scratch/scalability_test.cc -- --outpath="$outpath" --coreCloudNodeNum=$coreCloudNode
done
