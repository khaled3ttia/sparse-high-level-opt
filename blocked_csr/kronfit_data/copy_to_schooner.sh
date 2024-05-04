#!/bin/bash

# Source directory containing .mtx files
source_dir="/mnt/bulk/snap_kron"

# Destination server information
dest_user="khaled"
dest_server="dtn2.oscer.ou.edu"
dest_port="22"  # Change if the SSH port is different

# Destination directory where chunks will be copied
dest_dir="/scratch/khaled/snap_kron"

# Number of files per chunk
files_per_chunk=10

# Count total number of .mtx files
total_files=$(ls -1 "$source_dir"/*.mtx | wc -l)

# Calculate number of chunks
num_chunks=$(( (total_files + files_per_chunk - 1) / files_per_chunk ))

# Copy files to chunks
for (( i=0; i<num_chunks; i++ )); do
    # Create directory for chunk
    chunk_dir="$dest_dir/data_$((i+1))"
    mkdir -p "$chunk_dir"

    # Calculate start and end indices for files in this chunk
    start=$((i * files_per_chunk + 1))
    end=$(( (i+1) * files_per_chunk ))
    [ $end -gt $total_files ] && end=$total_files

    # Copy files to chunk directory
    files=$(ls -1 "$source_dir"/*.mtx | sed -n "${start},${end}p")
    scp -P "$dest_port" $files "$dest_user@$dest_server:$chunk_dir"
done
