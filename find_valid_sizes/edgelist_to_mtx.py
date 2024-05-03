import numpy as np
import pandas as pd
import scipy.sparse as sp
import scipy.io as spio
import math
import os
import argparse

def main(edgelist):
    data = pd.read_csv(edgelist, sep='\t',comment='#', names=['from','to'])

    basename = os.path.basename(edgelist)
    dirname = os.path.dirname(edgelist)

    basename_no_ext = os.path.splitext(basename)[0]
    if basename_no_ext.startswith('k'):
        k_val = int(basename_no_ext.split('_')[0].replace('k',''))
    else:
        k_val = int((basename_no_ext.split('.')[0]).split('_')[-1])

    nodes = int(math.pow(2, k_val))

    rows = data['from']
    cols = data['to']

    ones = np.ones(len(rows), np.float32)
    matrix = sp.coo_matrix((ones, (rows,cols)), (nodes,nodes))


    spio.mmwrite(f'{dirname}/{basename_no_ext}.mtx',matrix)


if __name__ == '__main__':
    parser =  argparse.ArgumentParser()

    parser.add_argument('--i', type=str, help='Input Edge List File Path', required=True)
    args = parser.parse_args()

    main(args.i)
