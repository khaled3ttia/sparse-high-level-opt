#!/bin/bash

if [ -z "$1" ]; then
    echo "Input data directory not provided!"
    exit
else
    input_dir="$1"
fi
for input_file in `ls ${input_dir}/*.mtx`;
do

    ./benchmark.x -i $input_file -r 4096 -c 4096 -o results/

done

