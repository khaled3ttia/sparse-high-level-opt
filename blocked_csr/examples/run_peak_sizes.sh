#!/bin/bash

if [ -z "$1" ]; then
    echo "Input data directory not provided!"
    exit
else
    input_dir="$1"
fi
for input_file in `ls ${input_dir}/*.mtx`;
do

    ./benchmark.x -i $input_file -w 8192 -u 8192 -r 262144 -c 262144 -o tiling_peak/

done

