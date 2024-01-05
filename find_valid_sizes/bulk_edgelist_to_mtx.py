import pandas as pd 
import glob 
import argparse 
from pathlib import Path
import dask.dataframe as dd
import os 
from dask.distributed import Client, LocalCluster 
import subprocess 


cmds_with_errs = [] 
cmd_errs = [] 
def run_cmd(row):
    cmds = [f"""python edgelist_to_mtx.py --i {row['filepath']}""", f"""rm {row['filepath']}"""]
    print(f"""processing {row['filepath']}...""")
    for cmd in cmds:
        output = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)
        if output.returncode != 0:
            print('Encountered error for some reason, check error_log.csv for details')
            cmds_with_errs.append(cmd)
            cmd_errs.append(output.stderr.decode('utf-8'))
    

def run_cmd_dask(df):
    return df.apply(run_cmd, axis=1)

def main(args):

    indir = args.indir 

    infiles = glob.glob(f'{indir}/*.txt')

    filepaths = pd.DataFrame(data={'filepath':infiles})
    

    nthreads = os.cpu_count()

    if len(filepaths) > 0:
        filepaths_d = dd.from_pandas(filepaths, npartitions=int(len(filepaths)/nthreads))
        filepaths_d.map_partitions(run_cmd_dask).compute(scheduler='threads')

        if len(cmds_with_errs) > 0:
            error_log = pd.DataFrame(data={'cmd':cmds_with_errs, 'cmd_errs': cmd_errs})
            error_log.to_csv('error_log.csv', index=False)

    
if __name__ == '__main__':

    parser = argparse.ArgumentParser() 


    parser.add_argument('--indir', type=str, default='./mtx_files', help="Input directory that contains edgelist files")

    args = parser.parse_args()
    main(args)
