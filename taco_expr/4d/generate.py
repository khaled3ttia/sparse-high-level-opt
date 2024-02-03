import dask.multiprocessing
import os 
import glob
import argparse
import scipy as sp 
import math
import os 
from pathlib import Path

def nearest_power_of_2(n):
    power = 1 
    while power < n:
        power *= 2 
    if abs(n - power // 2) <= abs(n - power):
        return power // 2 
    else:
        return power 

def process_mat(mtx):
    #base_file_name = mtx.split('.')[0]
    base_file_name = Path(mtx).stem
    print(base_file_name)
    directory = os.path.dirname(mtx)

    mat = sp.io.mmread(mtx)
    rows, cols = mat.shape

    row_nearest_power = nearest_power_of_2(rows)
    row_nearest_expo = int(math.log2(row_nearest_power))
    row_powers = set([2**x for x in range(1, row_nearest_expo+1)])

    col_nearest_power = nearest_power_of_2(cols)
    col_nearest_expo = int(math.log2(col_nearest_power))
    col_powers = set([2**x for x in range(1, col_nearest_expo+1)])

    for nb in row_powers:
        for mb in col_powers:
            print(f'Generating 4D tensor with dimension {nb} x {mb}')
            tensor_entries = []
            for i, j, v in zip(mat.row, mat.col, mat.data):
                io, ii, jo, ji = 0,0,0,0
                io = (i // nb) + 1
                ii = (i % nb) + 1
                jo = (j // mb) + 1
                ji = (j % mb) + 1
                #tensor_entries.append((io,ii,jo,ji,v))
                tensor_entries.append((io, jo, ii, ji))
            tensor_entries.sort()
            gen_tensor_path = f'{directory}/{base_file_name}-r{nb}_c{mb}.tns'
            with open(gen_tensor_path,'w') as f:
                for io, ii, jo, ji, v in tensor_entries:
                    f.write(f'{io} {ii} {jo} {ji} {v}\n')
            print(f'4D Tensor file generated at {gen_tensor_path}')

def main(args):
    
    num_cpus = os.cpu_count()

    dask.config.set(scheduler='processes', num_workers=num_cpus)


    futures = [] 
    files = glob.glob(f'{args.indir}/*.mtx')

    for file in files:
        futures.append(dask.delayed(process_mat)(file))

    dask.compute(futures)

if __name__ == '__main__':
    
    parser = argparse.ArgumentParser()
    parser.add_argument('--indir', type=str, required=True, help='Input directory containing 2D mtx files')
    args = parser.parse_args()

    main(args)


    
