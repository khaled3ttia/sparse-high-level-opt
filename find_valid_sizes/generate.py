import pandas as pd 
import glob
import argparse
from pathlib import Path
import subprocess 
#import swifter 
import dask.dataframe as dd
import os

def run_cmd(row, krongen,k_val,outdir):
    outfile = f"k{k_val}_{row['x0']:.3f}_{row['x1']:.3f}_{row['x2']:.3f}_{row['x3']:.3f}"
    cmd = f"""{krongen} -o:{outdir}/{outfile}.txt -m:"{row['x0']:.3f} {row['x1']:.3f}; {row['x2']:.3f} {row['x3']:.3f}" -i:{k_val} """
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
    

    def run_cmd_dask(df): return df.apply(run_cmd, args=(krongen,k_val,outdir,), axis=1)

    for file in files_to_process:
        df = pd.read_csv(file)
        k_val = int(f"{Path(file).stem.split('_')[1]}")
        to_generate = df[df['generate']].copy()
        count_generate = len(to_generate.index)
        
        to_generate_d = dd.from_pandas(to_generate, npartitions=int(count_generate/nthreads))
        if not to_generate.empty:
            print(f'to_generate : {to_generate}')
            print(len(to_generate))
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
