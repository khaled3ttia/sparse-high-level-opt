#!/bin/bash

if [ -z "$1" ]; then 
	echo "Input data directory not provided, using the default /data/"
	input_dir=/data
else
	input_dir="$1"
fi

if [ -z "$2" ]; then 
	echo "Output results file path not provided, using the default ./out_results_large.csv"
	results_file="out_results_large.csv"
else
	results_file="$2"
fi

if [ -z "$3" ]; then
	echo "Number of trials per experiment not provided, using a default of 10"
	num_trials=10
else
	num_trials="$3"
fi

base_cmd='taco "a(i) = B(i,j) * c(j)" -f=c:d -f=a:d -g=c:r -nthreads=`nproc` -time='"${num_trials}"' -write-time=/dev/stdout -i=B:'

echo "filename,dcsr_compile,dcsr_assemble,dcsr_mean,dcsr_stdev,dcsr_median" > $results_file


for input_file in `ls ${input_dir}/*.mtx`;
do
    base_mtx_name=$(basename $input_file)
    
    read -r rows cols nnz <<< `head -n 3 $input_file | tail -n 1`

    echo $base_mtx_name...
   
    prefix="${base_cmd}${input_file} -f=B:"

    echo "CSR DOUBLE NO BLOCKING"
    csr_cmd="(${prefix}ds)"
    csr_full_out=$(eval "${csr_cmd}")
    csr_out=$(echo "$csr_full_out" | tail -n 1)


    for i in 

    echo "$base_mtx_name,$csr_out" >> $results_file

done
