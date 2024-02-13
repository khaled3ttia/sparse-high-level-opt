#!/bin/bash
#
#SBATCH --partition=disc
#SBATCH --exclusive
#SBATCH --nodes=1
#SBATCH --ntasks=128
#SBATCH --mem=256G
#SBATCH --output=dask_single_node_%J_stdout.txt
#SBATCH --error=dask_single_node_%J_stderr.txt
#SBATCH --time=01:00:00
#SBATCH --job-name=dask_single_node
#SBATCH --mail-user=khaled.abdelaal@ou.edu
#SBATCH --mail-type=ALL
#SBATCH --chdir=/home/khaled/
#
#################################################
module load Python/3.9.5-GCCcore-10.3.0

cd /home/khaled/sparse-high-level-opt/find_valid_sizes

python generate.py --krongen /home/khaled/krongen/krongen --outdir /scratch/khaled/dask_out



