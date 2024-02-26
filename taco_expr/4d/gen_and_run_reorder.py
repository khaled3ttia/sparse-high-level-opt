#import dask.multiprocessing
import os 
import glob
import argparse
import scipy as sp 
import math
import os 
import subprocess 
from pathlib import Path

results_filename = 'taco_4d_ssss_iojo_reorder_k21.csv'
def process_mat(mtx, indir, outdir, start, end, copy_freq):
    #base_file_name = mtx.split('.')[0]
    base_file_name = Path(mtx).stem
    print(base_file_name)

    mat = sp.io.mmread(mtx)
    rows, cols = mat.shape
    

    count_files = 0 
    for nb in (2**index for index in range(start, end+1)):
        for mb in (2**index for index in range(start, end+1)):
            print(f'Generating 4D tensor with dimension {nb} x {mb}')
            tensor_entries = []
            for i, j, v in zip(mat.row, mat.col, mat.data):
                io, ii, jo, ji = 0,0,0,0
                io = (i // nb) + 1
                ii = (i % nb) + 1
                jo = (j // mb) + 1
                ji = (j % mb) + 1
                #tensor_entries.append((io,ii,jo,ji,v))
                tensor_entries.append((io, jo, ii, ji, v))
            tensor_entries.sort()
            gen_tensor_path = f'{outdir}/{base_file_name}-r{nb}_c{mb}.tns'
            with open(gen_tensor_path,'w') as f:
                #for io, ii, jo, ji, v in tensor_entries:
                for io, jo, ii, ji, v in tensor_entries:
                    #f.write(f'{io} {ii} {jo} {ji} {v}\n')
                    f.write(f'{io} {jo} {ii} {ji} {v}\n')

            print(f'4D Tensor file generated at {gen_tensor_path}')
            # Run apptainer and execute 
            
            # change current directory to /home/khaled and exec
            apptainer_cmd = f'apptainer exec --bind {outdir}:/data taco_py_latest.sif /data/reorder_run_tool_4d.sh /data/ /data/{results_filename} 10'
            print(f'Launching apptainer to evaluate the generated tensor...') 
            apptainer_process = subprocess.run(apptainer_cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, cwd='/home/khaled/', shell=True)

            if (apptainer_process.returncode == 0):
                count_files += 1 
                print(f'Apptainer process for {gen_tensor_path} exited successfully!')
                print(f'Deleting tensor file')


                delete_cmd = f'rm {gen_tensor_path}'
                delete_process = subprocess.run(delete_cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)
                if (delete_process.returncode == 0):
                    print('Deletion successful!')
                else:
                    print(f"Error deleting {delete_process.stderr.decode('utf-8')}")
                if (count_files % copy_freq == 0):
                    # Copy results file from $LSCRATCH to scratch
                    cp_cmd = f'cp {outdir}/{results_filename} {indir}'
                    cp_process = subprocess.run(cp_cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)
                    if (cp_process.returncode == 0):
                        print(f'Copied results file from $LSCRATCH to /scratch')
                    else:
                        print(f"Error copying results file {cp_process.stderr.decode('utf-8')}")
                
            else:
                print(f"Error with the apptainer process {apptainer_process.stderr.decode('utf-8')}")

def main(args):
    
    num_cpus = os.cpu_count()

    #dask.config.set(scheduler='processes', num_workers=num_cpus)


    futures = [] 
    files = glob.glob(f'{args.indir}/*.mtx')
    indir = args.indir 
    outdir = args.outdir
    start = args.start
    end = args.end 
    copy_freq = args.copy_freq

    # create output directory if it does not exist 
    if not os.path.exists(outdir):
        os.makedirs(outdir)

    for file in files:
        #futures.append(dask.delayed(process_mat)(file, indir, outdir, start, end, copy_freq))
        process_mat(file,indir,outdir, start,end,copy_freq)
    #dask.compute(futures)

    # copy results file for a final time 
    cp_cmd = f'cp {outdir}/{results_filename} {indir}'
    cp_process = subprocess.run(cp_cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)
    if (cp_process.returncode == 0):
        print(f'Copied final version of results file successfully!')
    else:
        print(f"Failed to copy results file {cp_process.stderr.decode('utf-8')}")

if __name__ == '__main__':
    
    parser = argparse.ArgumentParser()
    parser.add_argument('--indir', type=str, required=True, help='Input directory containing 2D mtx files')
    parser.add_argument('--outdir', type=str, required=True, help='Output directory for temporary 4D tns files')
    parser.add_argument('--start', type=int, required=True, help='Smallest Tile dimesnion Base 2 Power to evaluate. For example, for dimension of size 1024, you should use 10')
    parser.add_argument('--end', type=int, required=True, help='Biggest Tile dimension Base 2 Power to evaluate. For exmaple, for dimension size of 32, you should use 5')
    parser.add_argument('--copy_freq', type=int, required=True, default=10, help='Copy result file after each n tensor files being evaluatd successfully')

    args = parser.parse_args()

    

    main(args)


    
