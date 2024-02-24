#!/bin/bash

if [ -z "$1" ]; then 
	echo "Input data directory not provided, using the default /data/"
	input_dir=/data
else
	input_dir="$1"
fi

if [ -z "$2" ]; then 
	echo "Output results file path not provided, using the default ./out_results_large.csv"
	results_file="out_results_large_4d.csv"
else
	results_file="$2"
fi

if [ -z "$3" ]; then
	echo "Number of trials per experiment not provided, using a default of 10"
	num_trials=10
else
	num_trials="$3"
fi

base_cmd="taco \"a(i,j) = B(i,j,k,l) * c(k,l)\" -f=a:dd -f=c:dd -g=c:r -time="${num_trials}" -write-time=/dev/stdout -f=B:duuu -i=B:"
echo "$base_cmd"
echo "filename,dcsr_compile,dcsr_assemble,dcsr_mean,dcsr_stdev,dcsr_median" > $results_file

#l3_size=`getconf -a | grep LEVEL3_CACHE_SIZE | awk '{print $2}'`


for input_file in `ls ${input_dir}/*.tns`;
do
    base_tns_name=$(basename $input_file)

    echo "$base_tns_name..."

    final_cmd="${base_cmd}${input_file}"
    cmd_full_out=$(eval "$final_cmd")
    if [ $? -ne 0 ];
    then
	    echo "FAILED"
    fi
    cmd_out=$(echo "$cmd_full_out" | tail -n 1)
    echo "$base_tns_name,$cmd_out" >> $results_file

done

