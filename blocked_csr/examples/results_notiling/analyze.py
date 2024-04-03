import pandas as pd

import glob

import os

import matplotlib.pyplot as plt
import seaborn as sns

sns.set_theme(style='whitegrid')
def main():


    stats = pd.read_csv('matrices.stats')
    stats['filename'] = stats['filename'].str.split('/').str[-1]

    results_files = glob.glob('*.csv')

    results_dfs = {}

    df = pd.DataFrame()
    for file in results_files:
        base_file_name, _ = os.path.splitext(file)
        nnz = stats[stats['filename']==base_file_name]['nnz'].iloc[0]
        file_df = pd.read_csv(file)

        file_df['k'] = int(base_file_name.split('_')[0].replace('k',''))
        file_df['gflops'] = (2 * nnz) / (file_df['median'] * 1e6)

        #print(file_df)
        df = pd.concat([df, file_df], axis=0)
    df = df.sort_values(by='k', ascending=True)
    fig, ax = plt.subplots()
    ax.plot(df['k'], df['gflops'])
    ax.grid(True)
    ax.set_xticks(df['k'])
    ax.set_xticklabels(df['k'].apply(str))
    ax.set_xlabel('K Power')
    ax.set_ylabel('Median GFLOPs')
    fig.savefig('k_vs_gflops_mkl_notiling.png')
if __name__ == '__main__':
    main()

