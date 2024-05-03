#!/bin/bash

CONDA_ENV_NAME=sparse-high
conda activate $CONDA_ENV_NAME

ROOT_DIR="/home/khaled/Documents/sparse-high-level-opt"

GENERATE_SCRIPT_DIR="${ROOT_DIR}/blocked_csr/kronfit_data/mtx_files"
CONVERT_SCRIPT_DIR="${ROOT_DIR}/find_valid_sizes"

# first generate graphs
cd $GENERATE_SCRIPT_DIR
python generate.py

# then convert output to mtx files
cd ${CONVERT_SCRIPT_DIR}
python bulk_edgelist_to_mtx.py --indir $GENERATE_SCRIPT_DIR

