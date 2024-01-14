#!/bin/bash

#BLOCK_SIZE_THRESHOLD=512
nearest_power_of_two() {
  n=$1
  if (( n == 0 )); then
    echo 1
    return
  fi
  p=1
  while (( p < n )); do
    p=$(( p * 2 ))
  done
  if (( n - (p / 2) <= p - n )); then
    echo $(( p / 2 ))
  else
    echo $p
  fi
}

log2 () {
  local x=$1
  local y=0
  while (( x > 1 )); do
    x=$(( x / 2 ))
    y=$(( y + 1 ))
  done
  echo $y
}

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

base_cmd="taco \"a(i) = B(i,j) * c(j)\" -f=a:d -f=c:d -g=c:r -time="${num_trials}" -write-time=/dev/stdout -f=B:ds -i=B:"
echo "$base_cmd"
echo "filename,rows_per_block,cols_per_block,dcsr_compile,dcsr_assemble,dcsr_mean,dcsr_stdev,dcsr_median" > $results_file

#l3_size=`getconf -a | grep LEVEL3_CACHE_SIZE | awk '{print $2}'`


for input_file in `ls ${input_dir}/*.mtx`;
do
    base_mtx_name=$(basename $input_file)

    echo "$base_mtx_name..."
    read -r nrows ncols nnz <<< `head -n 3 $input_file | tail -n 1`

    nearest_row=$(nearest_power_of_two ${nrows})
    nearest_row_base=$(log2 ${nearest_row})
    nearest_col=$(nearest_power_of_two ${ncols})
    nearest_col_base=$(log2 ${nearest_col})

    row_powers=()
    for ((x=1; x<=nearest_row_base; x++)); do
	    row_powers+=($((2**x)))
    done
    col_powers=()
    for ((x=1; x<=nearest_col_base; x++)); do
	    col_powers+=($((2**x)))
    done
	
    prefix="${base_cmd}${input_file} -f=B:ds"
    echo "${prefix}"
    for nr in "${row_powers[@]}"; do
	for nc in "${col_powers[@]}"; do
		final_cmd="${prefix} -s=\"split(i,i0,ii,${nr}),split(j,j0,ji,${nc}),reorder(i0,j0,ii,ji)\""
		echo "$final_cmd"
	        
		cmd_full_out=$(eval "$final_cmd")
		if [ $? -ne 0 ];
		then
			echo "FAILED"
		fi
		cmd_out=$(echo "$cmd_full_out" | tail -n 1)
		echo "$base_mtx_name,$nr,$nc,$cmd_out" >> $results_file
	done
    done
done

