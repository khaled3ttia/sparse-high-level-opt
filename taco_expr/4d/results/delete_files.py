import pandas as pd 
import glob
from pathlib import Path 
import argparse 
import os
import os.path 

total_size_freed = 0 

def run_cmd(row,base_dir):

    filename = row['filename'] 
    full_file_path = f'{base_dir}/{filename}'
    print(f'Trying to delete {full_file_path}')
    if os.path.isfile(full_file_path):

        file_size = Path(full_file_path).stat().st_size / 1024 / 1024 

        cmd = f'rm {full_file_path}'

        delete_out = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE,shell=True)

        if delete_out.returncode == 0 :

            total_size_freed += file_size 
            return 0
    else:
        return -1

def main(args):


    csv_file = args.infile
    base_dir = os.path.dirname(csv_file)


    results = pd.read_csv(csv_file)
    results.apply(run_cmd, args=(base_dir,), axis=1)

    total_size_free_gb = total_size_freed / 1024 
    print(f'Freed {total_size_free_gb} GBs!')

        #print(results)

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--infile', type=str, required=True, help="Input csv file the contains evaluation results")
    args = parser.parse_args()
    main(args)
