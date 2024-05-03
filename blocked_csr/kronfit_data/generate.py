import pandas as pd
import glob
import argparse
from pathlib import Path
import subprocess
#import swifter
import dask.dataframe as dd
import os
from dask.distributed import Client, LocalCluster

def run_cmd(row, krongen, min_k, max_k, outdir):
    count_errors = 0
    for k in range(min_k, max_k + 1):
        outfile = f"{row['Network']}_{k}"
        cmd = f"""{krongen} -o:{outdir}/{outfile}.txt -m:"{row['x0']:.3f} {row['x1']:.3f}; {row['x2']:.3f} {row['x3']:.3f}" -i:{k} """
        print(cmd)
        result_gen = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)
        if result_gen.returncode != 0:
            count_errors += 1
    return count_errors

'''
def run_cmd_dask(df):
    return df.apply(run_cmd, args=(krongen,k_val,outdir,), axis=1)
'''

def main(args):

    nthreads = 16

    indir = args.indir
    krongen = args.krongen
    outdir = args.outdir
    os.makedirs(outdir, exist_ok=True)
    snap_csv_file = args.snap_file
    min_k = args.min_k
    max_k = args.max_k

    cluster = LocalCluster()
    client = Client(cluster)

    def run_cmd_dask(df): return df.apply(run_cmd, args=(krongen,min_k,max_k,outdir,), axis=1)

    full_to_generate = pd.read_csv(snap_csv_file)
    print(full_to_generate.head())
    count_generate = len(full_to_generate.index)
    print(f'generating {count_generate} graphs using {nthreads}...')


    to_generate_d = dd.from_pandas(full_to_generate, npartitions=int(count_generate/nthreads))

    # Dask need a meta parameter, read more about it in dask documentation
    meta = pd.Series(dtype='int')
    print(to_generate_d.map_partitions(lambda df: df.head()).compute())
    if not full_to_generate.empty:
        result = to_generate_d.map_partitions(run_cmd_dask, meta=meta).compute(scheduler='threads')
        print(f'Number of errors in generation = {result}/{count_generate}')
if __name__ == '__main__':

    parser = argparse.ArgumentParser()

    parser.add_argument('--indir', type=str, default='.', help="Input directory for stats files")
    parser.add_argument('--krongen', type=str, default='/home/khaled/Documents/snap/examples/krongen/krongen', help="Full path for krongen binary (including the binary)")
    parser.add_argument('--outdir', type=str, default='./mtx_files', help="Directory to store output mtx files")
    parser.add_argument('--snap_file', type=str, default='kronvals.csv', help='Path to csv file containing snap estimated kronecker values on the format: Network, x0, x1, x2, x3')
    parser.add_argument('--min_k', type=int, default=1, help='Minimum Kronecker Power')
    parser.add_argument('--max_k', type=int, default=20, help='Maximum Kronecker Power')
    args = parser.parse_args()
    main(args)
