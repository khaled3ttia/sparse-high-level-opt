import pandas as pd

import glob

import os

import matplotlib.pyplot as plt
import seaborn as sns
from mpl_toolkits.mplot3d import Axes3D

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
        file_df['Tile Size'] = file_df['rows'].astype(str) + 'x' + file_df['cols'].astype(str)

        fig = plt.figure()
        ax = fig.add_subplot(111, projection='3d')
        sc = ax.scatter(file_df['rows'], file_df['cols'], file_df['gflops'], c=file_df['gflops'], cmap='viridis')
        plt.colorbar(sc, label='GFLOPS')
        ax.set_xlabel('Rows per Tile')
        ax.set_ylabel('Cols per Tile')
        ax.set_zlabel('GFLOPS')
        fig.savefig(f'{file}_tile_perf.png')
        plt.close(fig)

        file_df.sort_values(['rows','cols'], inplace=True)
        file_df['Tile Size'] = file_df['Tile Size'].replace('0x0', 'No Tiling')
        bar_positions = range(len(file_df['Tile Size']))
        bar_width = 0.6
        fig, ax = plt.subplots(figsize=(15,10))
        #fig, ax = plt.subplots()
        ax.bar(bar_positions, file_df['gflops'], width=bar_width)
        ax.grid(True, which='both', linestyle='--', linewidth=0.5)
        ax.set_xlabel('Tile Size (rows x cols)')
        ax.set_ylabel('Performance (GFLOPS)')
        ax.set_xticks(range(len(file_df['Tile Size'])))
        ax.set_xticklabels(file_df['Tile Size'], rotation=90)

        file_df = file_df.sort_values(by='gflops', ascending=False)
        file_df.to_csv(f'{file}_sorted_gflops.csv', index=False)
        fig.savefig(f'{file}_tile_bar_perf.png', bbox_inches='tight')

if __name__ == '__main__':
    main()

