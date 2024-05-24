import pandas as pd

import glob

import os

import matplotlib.pyplot as plt
import seaborn as sns
from mpl_toolkits.mplot3d import Axes3D
import itertools 

sns.set_theme(style='whitegrid')
def main():


    stats = pd.read_csv('snap_kron.stats')
    stats['filename'] = stats['filename'].str.split('/').str[-1]

    results_files = glob.glob('*.csv')

    results_dfs = {}

    df = pd.DataFrame()
    
    snap_graphs = {}    

    for file in results_files:
        base_file_name, _ = os.path.splitext(file)
        snap_graph = base_file_name.split('.')[0].split('_')[0]
        if snap_graph not in snap_graphs:
            #snap_graphs[snap_graph] = pd.DataFrame(columns=['k','gflops'])
            snap_graphs[snap_graph] = pd.DataFrame()
        nnz = stats[stats['filename']==base_file_name]['nnz'].iloc[0]
        print(f'parsing {file}')
        file_df = pd.read_csv(file)
        #file_df['k'] = int(base_file_name.split('_')[0].replace('k',''))
        k = int(base_file_name.split('.')[0].split('_')[-1])
        file_df['k'] = k
        file_df['gflops'] = (2 * nnz) / (file_df['median'] * 1e6)
        file_df['Tile Size'] = file_df['rows'].astype(str) + 'x' + file_df['cols'].astype(str)

        file_df.sort_values(['rows','cols'], inplace=True)
        file_df['Tile Size'] = file_df['Tile Size'].replace('0x0', 'No Tiling')
        baseline_gflops = file_df.loc[file_df['Tile Size'] == 'No Tiling', 'gflops'].iloc[0]
        graph_entry = pd.DataFrame({'k':[k], 'gflops':[baseline_gflops]})
        #graph_entry = {'k': file_df['k'], 'gflops': baseline_gflops}
	
        #snap_graphs[snap_graph].loc[len(snap_graphs[snap_graph])] = graph_entry
        snap_graphs[snap_graph] = pd.concat([snap_graphs[snap_graph], graph_entry], ignore_index=True)

        #bar_positions = range(len(file_df['Tile Size']))
        #bar_width = 0.6
        #fig, ax = plt.subplots(figsize=(15,10))
        #fig, ax = plt.subplots()
        #ax.bar(bar_positions, file_df['gflops'], width=bar_width)
        #ax.grid(True, which='both', linestyle='--', linewidth=0.5)
        #ax.set_xlabel('Tile Size (rows x cols)')
        #ax.set_ylabel('Performance (GFLOPS)')
        #ax.set_xticks(range(len(file_df['Tile Size'])))
        #ax.set_xticklabels(file_df['Tile Size'], rotation=90)

        #file_df = file_df.sort_values(by='gflops', ascending=False)
        #file_df.to_csv(f'{file}.sorted', index=False)
        #fig.savefig(f'{file}_tile_bar_perf.png', bbox_inches='tight')
        #plt.close(fig)
    
    colors = plt.cm.get_cmap('tab10', len(snap_graphs))
    line_styles = itertools.cycle(['-', '--', '-.', ':']) 
    fig, ax = plt.subplots(figsize=(10,6))
    #print(snap_graphs)
    for i, (graph, df) in enumerate(snap_graphs.items()):
        #print(df)
        #df['k'] = pd.to_numeric(df['k'])
        df_sorted = df.sort_values(by='k')
        ax.plot(df_sorted['k'], df_sorted['gflops'], label=graph, color=colors(i), linestyle=next(line_styles))

    ax.set_xlabel('K Power')
    ax.set_ylabel('Performance (GFLOPs)')
    ax.set_xticks(range(1,21))
    ax.legend(loc='upper left', bbox_to_anchor=(1,1))
    ax.grid(True)
    fig.savefig('k_vs_gflops_snap.png', bbox_inches='tight')
    plt.close(fig)
if __name__ == '__main__':
    main()

