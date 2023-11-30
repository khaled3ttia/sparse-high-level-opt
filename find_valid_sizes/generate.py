import pandas as pd 
import glob
import argparse
from pathlib import Path
import subprocess 
#import swifter 
import dask.dataframe as dd
import os
from dask.distributed import Client, LocalCluster

def run_cmd(row, krongen,outdir):
    outfile = f"k{row['k']}_{row['x0']:.3f}_{row['x1']:.3f}_{row['x2']:.3f}_{row['x3']:.3f}"
    cmd = f"""{krongen} -o:{outdir}/{outfile}.txt -m:"{row['x0']:.3f} {row['x1']:.3f}; {row['x2']:.3f} {row['x3']:.3f}" -i:{row['k']} """
    print(cmd)
    result_gen = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)

    return result_gen.returncode

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
    files_to_process = glob.glob(f'{indir}/*.stats')
    
    cluster = LocalCluster()
    client = Client(cluster)

    def run_cmd_dask(df): return df.apply(run_cmd, args=(krongen,outdir,), axis=1)

    full_to_generate = pd.DataFrame()
    for file in files_to_process:
        df = pd.read_csv(file)
        k_val = int(f"{Path(file).stem.split('_')[1]}")
        to_generate = df[df['generate']].copy()
        full_to_generate = pd.concat([full_to_generate,to_generate],ignore_index=True)
    count_generate = len(full_to_generate.index)
    print(f'generating {count_generate} graphs using {nthreads}...')

    to_generate_d = dd.from_pandas(full_to_generate, npartitions=int(count_generate/nthreads))
    if not full_to_generate.empty:
        #to_generate['exit_code'] = to_generate.swifter.set_dask_scheduler('processes').set_npartitions(16).allow_dask_on_strings(enable=True).apply(run_cmd, args=(krongen,k_val,outdir,), axis=1)
        to_generate_d.map_partitions(run_cmd_dask).compute(scheduler='threads')  
        #print(to_generate['exit_code']) 

if __name__ == '__main__':

    parser = argparse.ArgumentParser()

    parser.add_argument('--indir', type=str, default='.', help="Input directory for stats files")
    parser.add_argument('--krongen', type=str, default='/home/khaled/Documents/snap/examples/krongen/krongen', help="Full path for krongen binary (including the binary)")
    parser.add_argument('--outdir', type=str, default='./mtx_files', help="Directory to store output mtx files")

    args = parser.parse_args()
    main(args)
