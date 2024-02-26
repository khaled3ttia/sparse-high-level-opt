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
#SBATCH --output=gen_and_eval_stdout.txt
#SBATCH --error=gen_and_eval_stderr.txt
#SBATCH --job-name=gen_and_eval
##############################
module load Python/3.9.5-GCCcore-10.3.0

indir=/scratch/khaled/mtx_files

# Create the output dir 
outdir=$LSCRATCH/tns_4d_out
mkdir -p $outdir

results_file=$outdir/taco_4d_ssss_iojo_k21.csv
echo "filename,dcsr_compile,dcsr_assemble,dcsr_mean,dcsr_stdev,dcsr_median" > $results_file


# Copy the bash script for evaluation
cp /home/khaled/sparse-high-level-opt/taco_expr/4d/mod_run_tool_4d.sh $outdir 

# Run the python script 
cd /home/khaled/sparse-high-level-opt/taco_expr/4d

python gen_and_run.py --indir $indir --outdir $outdir --start 2 --end 12 --copy_freq 10 

lscpu
