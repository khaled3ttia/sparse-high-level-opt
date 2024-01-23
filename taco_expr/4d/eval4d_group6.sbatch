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
#SBATCH --output=eval4d_group6_stdout.txt
#SBATCH --error=eval4d_group6_stderr.txt
#SBATCH --job-name=eval4d_group6
##############################
cp /home/khaled/sparse-high-level-opt/taco_expr/4d/run_tool_4d.sh /scratch/khaled/mtx_group6
echo "Starting Apptainer Container..."
apptainer exec --bind /scratch/khaled/mtx_group6:/data taco_py_latest.sif /data/run_tool_4d.sh /data/ /data/taco_4d_uuud_group6.csv 10
echo "CPU SPEC"
lscpu
