#!/bin/bash
#
#SBATCH --partition=disc
#SBATCH --exclusive
#SBATCH --nodes=1
#SBATCH --ntasks=128
#SBATCH --mem=256G
#SBATCH --output=/home/khaled/sparse-high-level-opt/blocked_csr/tiling_peak_mkl_stdout.txt
#SBATCH --error=/home/khaled/sparse-high-level-opt/blocked_csr/tiling_peak_mkl_stderr.txt
#SBATCH --time=12:00:00
#SBATCH --job-name=tiling_peak_mkl
#SBATCH --mail-user=khaled.abdelaal@ou.edu
#SBATCH --mail-type=ALL
#SBATCH --chdir=/home/khaled/
#
#################################################

cpu_vendor=$(lscpu | grep "Vendor ID:" | awk '{print $3}')
echo "CPU Vendor is: ${cpu_vendor}"

if [ "$cpu_vendor" != "GenuineIntel" ]; then
    echo "Cancelling the job because CPU vendor is not Intel.:("
    scancel $SLURM_JOB_ID
fi
module load intel/2023a
#source "$ONEAPI/setvars.sh"

source /opt/oscer/software/imkl/2023.1.0/setvars.sh

module load GCC


cd /home/khaled/sparse-high-level-opt/blocked_csr/examples


make

./run_peak_sizes.sh /scratch/khaled/peak_tile

lscpu

#mkdir -p ~/cpu_sys_info
#cd ~/cpu_sys_info
#lscpu > mkl_cpu_info.txt
#lsmem > mem_info.txt

