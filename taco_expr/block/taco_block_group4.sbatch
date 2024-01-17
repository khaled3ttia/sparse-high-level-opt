#!/bin/bash
#
#SBATCH --exclusive
#SBATCH --partition=disc
#SBATCH --nodes=1
#SBATCH --ntasks=128
#SBATCH --mem=256G
#SBATCH --time=48:00:00
#SBATCH --mail-user=khaled.abdelaal@ou.edu
#SBATCH --mail-type=ALL
#SBATCH --chdir=/home/khaled
#SBATCH --output=taco_blocking_group4_%J_stdout.txt
#SBATCH --error=taco_blocking_group4_%J_stderr.txt
#SBATCH --job-name=taco_block_group4
##############################
cp /home/khaled/sparse-high-level-opt/taco_expr/block/run_tool_blocking.sh /scratch/khaled/mtx_group4
echo "Starting Apptainer Container..."
apptainer exec --bind /scratch/khaled/mtx_group4:/data taco_py_latest.sif /data/run_tool_blocking.sh /data/ /data/taco_block_group4.csv 10
echo "CPU SPEC"
lscpu
