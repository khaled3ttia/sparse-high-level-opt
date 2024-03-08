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
#SBATCH --output=gen_and_eval_reorder_manual_k16_stdout.txt
#SBATCH --error=gen_and_eval_reorder_manual_k16_stderr.txt
#SBATCH --job-name=gen_and_eval_reorder_k16
##############################
module load Python/3.9.5-GCCcore-10.3.0

indir=/scratch/khaled/mtx_files

# Create the output dir 
outdir=$LSCRATCH/tns_4d_out
mkdir -p $outdir

# TODO : Change per experiment
results_filename=taco_4d_ssss_iojo_reorder_manual_k16.csv

results_file=$outdir/$results_filename
echo "filename,dcsr_compile,dcsr_assemble,dcsr_mean,dcsr_stdev,dcsr_median" > $results_file


# Copy the bash script for evaluation
cp /home/khaled/sparse-high-level-opt/taco_expr/4d/reorder_manual_run_tool_4d.sh $outdir 

# Run the python script 
cd /home/khaled/sparse-high-level-opt/taco_expr/4d

python gen_and_run_reorder_manual.py --indir $indir --outdir $outdir --start 2 --end 11 --copy_freq 1 --outfile $results_filename 

lscpu
