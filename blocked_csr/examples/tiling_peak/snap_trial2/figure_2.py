import os
import re
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
from matplotlib.ticker import MultipleLocator, MaxNLocator

import argparse

# Function to extract dataset name and k value from the filename
def parse_filename(filename):
    match = re.match(r"(.*)_([0-9]+)\.mtx\.csv", filename)
    if match:
        dataset_name = match.group(1)
        k_value = int(match.group(2))
        return dataset_name, k_value
    return None, None


def loadStats(stats_file):
    df = pd.read_csv(stats_file)
    df['base_filename'] = df['filename'].apply(lambda x: os.path.basename(x))
    return df

def getNNZ(stats_df, base_filename):
    nnz = stats_df[stats_df['base_filename'] == base_filename]['nnz'].iloc[0]
    return nnz

def main(args):
   # Directory where the CSV files are stored
    directory = args.i

    # Create output directories if they don't exist
    if not os.path.exists(args.o):
        os.makedirs(args.o)

    stats_file = args.s
    stats_df = loadStats(stats_file)

    # Dictionary to store data for each dataset
    datasets = {}

    # Read all files in the directory
    for filename in os.listdir(directory):
        if filename.endswith(".csv"):
            dataset_name, k_value = parse_filename(filename)
            if k_value < 16:
                continue
            if dataset_name and k_value:
                file_path = os.path.join(directory, filename)
                data = pd.read_csv(file_path)

                # Add the k_value column to the dataframe
                data['k'] = k_value
                nnz = getNNZ(stats_df, f'{dataset_name}_{k_value}.mtx')
                data['gflops'] = (2 * nnz) / (data['median'] * 1e6)
                # Store the data in the dataset dictionary
                if dataset_name not in datasets:
                    datasets[dataset_name] = []
                datasets[dataset_name].append(data)

    # Function to offset labels to avoid overlap
    def offset_label(x, y, label, ax, offset=0.5):
        ax.text(x, y + offset, label, fontsize=8, ha='left')

    
    # Plotting
    for dataset_name, data_list in datasets.items():
        # Concatenate all dataframes for the dataset
        df = pd.concat(data_list)
        
        # Sort by k values for consistent plotting
        df = df.sort_values(by='k')
        
        # Group the data by 'rows' and 'cols' configuration
        configurations = df.groupby(['rows', 'cols'])

        
        # Create a dictionary to store the maximum GFLOPS of each configuration
        config_gflops_max = {}
        for (rows, cols), group in configurations:
            max_gflops = group['gflops'].max()
            config_gflops_max[(rows, cols)] = max_gflops

        # Sort the configurations by their maximum GFLOPS
        sorted_configs = sorted(config_gflops_max.items(), key=lambda x: x[1], reverse=True)

        # Select the top configurations, but ensure 'baseline' is always included
        top_configs = []
        baseline_config = (0, 0)
        if baseline_config in config_gflops_max:
            top_configs.append(baseline_config)
        
        # Add top N configurations excluding baseline, based on the argument value
        top_configs += [config for config, _ in sorted_configs if config != baseline_config][:args.n]

        fig, ax = plt.subplots()

        # Define color, line style, and marker combinations
        colors = ['b', 'g', 'r', 'c', 'm', 'y', 'k']
        line_styles = ['-', '--', '-.', ':']
        markers = ['o', 's', 'D', '^', 'v', '<', '>', 'p', '*']
        style_index = 0

        for (rows, cols) in top_configs:
            group = configurations.get_group((rows, cols))

            # Define label for configuration
            if rows == 0 and cols == 0:
                label = "No Tiling"
            else:
                label = f'{rows}x{cols}'

            # Choose color, line style, and marker combination
            color = colors[style_index % len(colors)]
            linestyle = line_styles[style_index % len(line_styles)]
            marker = markers[style_index % len(markers)]

            # Plot each configuration with k on the x-axis and gflops on the y-axis
            ax.plot(group['k'], group['gflops'], linestyle=linestyle, color=color, marker=marker, label=label)

            # Add inline label near the last point of the line
            #last_point = group.iloc[-1]
            #ax.text(last_point['k'], last_point['gflops'], label, fontsize=8, ha='left', va='center')

            # Increment style index to use different line styles, colors, and markers
            style_index += 1
        #ax.set_ylim(0, max(config_gflops_max.values()))
        #ax.set_ylim(bottom=0)
        #ax.yaxis.set_major_locator(MultipleLocator(0.2))
        #ax.yaxis.set_minor_locator(MultipleLocator(0.05))
        #ax.minorticks_on()

        # Set x-axis to display only integer values
        ax.xaxis.set_major_locator(MaxNLocator(integer=True))

        ax.set_title(f'Performance for {dataset_name}')
        ax.set_xlabel('k')
        ax.set_ylabel('Performance (GFLOPs)')
        ax.grid(True)
        ax.legend()

        # Save the figure to a file
        plt.savefig(f'{args.o}/{dataset_name}_gflops_linear.png', bbox_inches='tight')
        plt.close()  # Close the plot after saving it to a file 
if __name__ == '__main__':

    '''
        Sample Usage
        figure_2.py --i ./ --s ../snap/snap_kron.stats --n 10 --o ./output_directory
    '''

    parser = argparse.ArgumentParser()
    parser.add_argument('--i', type=str, help='Input Directory containing csv files ')
    parser.add_argument('--s', type=str, help='Stats File Path')
    parser.add_argument('--o', type=str, help='Output directory')
    parser.add_argument('--n', type=int, default=10, help='Number of top configurations to plot (excluding baseline)')
    args = parser.parse_args()

    main(args)
