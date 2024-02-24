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
#SBATCH --output=gen_and_eval_.txt
#SBATCH --error=eval4d_group1_stderr.txt
#SBATCH --job-name=eval4d_group1
##############################
module load Python/3.9.5-GCCcore-10.3.0

indir=/scratch/khaled/mtx_files

# Create the output dir 
outdir=$LSCRATCH/tns_4d_out
mkdir -p $outdir

# Copy the bash script for evaluation
cp /home/khaled/sparse-high-level-opt/taco_expr/4d/mod_run_tool_4d.sh $outdir 

# Run the python script 
cd /home/khaled/sparse-high-level-opt/taco_expr/4d

python gen_and_run.py --indir $indir --outdir $outdir --start 2 --end 12 --copy_freq 10 

lscpu
