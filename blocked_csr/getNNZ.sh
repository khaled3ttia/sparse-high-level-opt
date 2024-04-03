#!/bin/bash

# First argument is the input dir path. This dir should contain both the csv file (results)
# 	and all the mtx that are reffered to in the csv (results) file
#	exmaple : input_dir="/scratch/khaled/k21_same"
input_dir="$1"

# Second argument is the results (csv) file name ONLY. Must be in the input dir (input dir
# 	will be attached as the path prefix
#	exmaple value: taco_disc_k21.csv
#csv_file="${input_dir}/$2"


# Third argument is the required output stats file. This will be a newly created file. Path
# 	can be anywhere. It is recommended to have the extension .stats (to differentiate between
#	csv files and stats files in my experiments)
#	example value: /scratch/khaled/k21_same/graph_props.stats
stats_file="$2"

echo "filename,nnz,rows,cols,density" > $stats_file

#if [ ! -f "$csv_file" ]; then 
#	echo "CSV File not found: $csv_file"
#	exit 1
#fi

first_line=true
for filename in `ls ${input_dir}/*.mtx`; do
	line_of_interest=$(head -n3 "${filename}" | tail -n1)
	nnz=$(echo "${line_of_interest}" | awk '{print $3}')
	rows=$(echo "${line_of_interest}" | awk '{print $1}')
	cols=$(echo "${line_of_interest}" | awk '{print $2}')
	density=$(echo "scale=6; $nnz / ($rows * $cols)" | bc)
	#nnz=$(head -n3 "${input_dir}/${filename}" | tail -n1 | awk '{print $3}')
	echo "$filename,$nnz,$rows,$cols,$density" >> $stats_file 
done
