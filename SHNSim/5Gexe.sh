#!/bin/bash
export THIS=$(readlink -f 5G-Self-Organized-Network-Simulator-master/SHNSim) #replace lines 2 and 3 with 'cd [your filepath]'
cd $THIS
make clean
make output
make clean
chmod u+x output
./output
