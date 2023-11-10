import math
import pandas as pd 
import subprocess 
from pathlib import Path
import argparse 

def find_l3_cache_size():
    ''' finds L3 cache size in MB '''

    l3_size = -1
    cmd = """lscpu | grep 'L3 cache' | awk -F: '{print $2}' | awk '{print $1}' | sed 's/[^0-9.]//g'"""
    result = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)
    try:
        l3_size = int(result.stdout.decode('utf-8'))
    except:
        print('Cannot get the L3 cache size because of the following error!')
        print(result.stderr.decode('utf-8'))
    return l3_size 


def try_config(init_vals, kpower):

   num_nodes = int(math.pow(2, kpower))
   num_edges = int(math.pow(sum(init_vals), kpower))

   return num_nodes, num_edges

def coo_size(num_nodes, num_edges):
    # Assuming double takes 8 bytes and int takes 4 bytes
    vals_size = num_edges * 8 
    indices_size = num_edges * 2 * 4

    # in bytes
    total_size = vals_size + indices_size 

    total_size /= (1024 * 1024)

    # in MB
    return total_size

def csr_size(num_nodes, num_edges):

    vals_size = num_edges * 8 
    cols_size = num_edges * 4
    rows_size = (num_nodes + 1) * 4 

    total_size = vals_size + cols_size + rows_size 

    total_size /= (1024 * 1024)

    return total_size

def csc_size(num_nodes, num_edges):
    # because num_rows = num_cols 
    return csr_size(num_nodes, num_edges)

def fits_in_cache(cache_size, input_csv, outdir, max_file_size=200):
    ''' tests whether a specific configuration fits in L3 cache or not, and adds 
        new columns to the input csv file to reflect these information

        params:

            cache_size: L3 cache size in MB
            input_csv: the input csv file (generated by `generate_stats_files`)
            max_file_size: the desired maximum file size for a matrix
    '''
    
    # Read input csv file 
    df = pd.read_csv(input_csv)

    # Check for each sparse format
    df['coo_fits'] = df['coo_size'].apply(lambda x: x <= cache_size)
    df['csr_fits'] = df['csr_size'].apply(lambda x: x <= cache_size)
    df['csc_fits'] = df['csc_size'].apply(lambda x: x <= cache_size)
    df['all_fit'] = df.apply(lambda x: all(x[['coo_fits', 'csr_fits', 'csc_fits']]), axis=1)

    # Generate is True if none of the formats fit in cache, and the coo_size is less than
    # or equal to the pre-defined maximum file size
    df['generate'] = (df['all_fit'] == False) & (df['coo_size'] <= max_file_size)

    # Save to a new file 
    df.to_csv(f'{Path(input_csv).stem}_cache_{cache_size}.stats')




def generate_stats_files(init_vals, min_k, max_k, steps_per_val, init_val_delta, outdir):
    ''' Generates csv files that capture the following informations
        x0 : initiator matrix value 0 
        x1 : initiator matrix value 1 
        x2 : initiator matrix value 2
        x3 : initiator matrix values 3
        num_nodes: number of nodes (or rows or cols) in the generated graph
        num_edges: number of edges (or non-zeros) in the generated graph
        coo_size: size of the graph/matrix in COO format in memory
        csr_size: size of the graph/matrix in CSR format in memory 
        csc_size : size of the graph/matrix in CSC format in memory

        each file will be generated in the same directory, and will have the name k_{i}.csv 
        where i is the current Kronecker power being tested

        params:
            init_vals: a 4-value list contiaining the initiator matrix vlaues
            min_k: the minimum value of k Power to be tested
            max_k: the maximum value of k Power to be tested
            steps_per_val: how many steps (changes) to test for each single value of init_vals
            init_val_delta: how much a step is for each single value of init_vals
            outdir: output directory path where the files will be generated
    '''

    generated_files_paths = [] 

    for i in range(min_k, max_k + 1):
        init_vals_new = init_vals.copy()
        curr_file_path = f'{outdir}/k_{i}.csv'
        generated_files_paths.append(curr_file_path)
        with open(curr_file_path, 'w') as f:
            f.write(f'x0,x1,x2,x3,num_nodes,num_edges,coo_size,csr_size,csc_size\n')
        print(f'K = {i}')
        for j in range(len(init_vals_new)):
            for s in range(steps_per_val):
                if j == 0:
                    init_vals_new[j] -= init_val_delta
                else:
                    init_vals_new[j] += init_val_delta
                num_nodes, num_edges = try_config(init_vals_new, i)
                coo_size_ = coo_size(num_nodes, num_edges)
                csr_size_ = csr_size(num_nodes, num_edges)
                csc_size_ = csc_size(num_nodes, num_edges)

                with open(curr_file_path, 'a') as f:
                    f.write(f'{init_vals_new[0]:.3f},{init_vals_new[1]:.3f},{init_vals_new[2]:.3f},{init_vals_new[3]:.3f},{num_nodes},{num_edges},{coo_size_},{csr_size_},{csc_size_}\n')
                
    return generated_files_paths

def main(args):
    init_mat = [0.999, 0.437, 0.437, 0.484]
    min_k = args.min_k
    max_k = args.max_k
    steps_per_val = args.steps_per_val
    init_val_delta = args.init_val_delta
    outdir = args.outdir
    max_file_size = args.max_file_size
    files = generate_stats_files(init_mat,min_k,max_k,steps_per_val,init_val_delta,outdir)
    
    l3_size = find_l3_cache_size()
    print(f'l3 cache size is {l3_size} MB')

    for file in files: 
        fits_in_cache(l3_size,file, outdir, max_file_size)


if __name__ == '__main__':
    parser = argparse.ArgumentParser()

    parser.add_argument('--outdir', type=str, default='.', help="Output directory to store generated files")
    parser.add_argument('--max_file_size', type=int, default=100, help="Desired maximum file size for generated matrix/graph")
    parser.add_argument('--min_k', type=int, default=2 , help="Minimum Kronecker Power value to evaluate")
    parser.add_argument('--max_k', type=int, default=22, help="Maximum Kronecker Power value to evaluate")
    parser.add_argument('--init_val_delta', type=float, default=0.001, help="Delta (step) for each initiator matrix value to evaluate")
    parser.add_argument('--steps_per_val', type=int, default=100, help="Number of delta steps to evaluate per initiator matrix value")
    
    args = parser.parse_args()

    main(args)
