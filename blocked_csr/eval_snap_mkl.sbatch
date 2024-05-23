#!/bin/bash
#SBATCH --partition=disc
#SBATCH --exclusive
#SBATCH --nodes=1
#SBATCH --ntasks=128
#SBATCH --mem=256G
#SBATCH --output=/home/khaled/sparse-high-level-opt/blocked_csr/tiling_snap_mkl_stdout_%a.txt
#SBATCH --error=/home/khaled/sparse-high-level-opt/blocked_csr/tiling_snap_mkl_stderr_%a.txt
#SBATCH --time=12:00:00
#SBATCH --job-name=tiling_snap_mkl
#SBATCH --mail-user=khaled.abdelaal@ou.edu
#SBATCH --mail-type=ALL
#SBATCH --chdir=/home/khaled/
#SBATCH --array=1-40%5 # Run 5 tasks at a time

cpu_vendor=$(lscpu | grep "Vendor ID:" | awk '{print $3}')
echo "CPU Vendor is: ${cpu_vendor}"

if [ "${cpu_vendor}" != "GenuineIntel" ]; then
    echo "Cancelling the job because CPU vendor is not Intel. :("
    scancel $SLURM_JOB_ID
fi

module load intel/2023a
# source "$ONEAPI/setvars.sh"
source /opt/oscer/software/imkl/2023.1.0/setvars.sh
module load GCC

cd /home/khaled/sparse-high-level-opt/blocked_csr/examples
make

# Calculate input directory based on task array index
input_dir="/scratch/khaled/blocked/data_${SLURM_ARRAY_TASK_ID}"

# Run the script with the calculated input directory
./run_peak_sizes.sh "$input_dir"

lscpu

