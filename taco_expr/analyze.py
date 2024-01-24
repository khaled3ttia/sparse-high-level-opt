import pandas as pd 
import glob
import math


# Given a tensor file name, extracts block size and original mtx filename
def split(row):
    
    # Example: 'k16_0.899_0.537_0.548_c862-r1024_c1024.tns'

    org_filename = row['filename']

    # ['k16_0.899_0.537_0.548_c862','r1024_c1024.tns']
    new_name = org_filename.split('-')

    # ['r1024_c1024','tns']
    config = new_name[1].split('.')[0]

    # ['r1024','c1024']
    blocks = config.split('_')

    row['rows'] = int(blocks[0][1:])
    row['cols'] = int(blocks[1][1:])
    row['filename'] = f'{new_name[0]}.mtx'

    return row



def main():
    noblock = pd.read_csv('noblock/results/taco_noblock.csv')

    block_result_files = glob.glob('block/results/taco_block_group?.csv')

    tensor_result_files = glob.glob('4d/results/taco_4d_*.csv')


    # load csv files into dataframes
    block = pd.concat(map(pd.read_csv, block_result_files), ignore_index=True)
    tensor = pd.concat(map(pd.read_csv,tensor_result_files), ignore_index=True)
    
    # Remove NaN from 4D
    tensor = tensor.dropna()
    
    # Extract block sizes from file name and store only base mtx filename 
    tensor = tensor.apply(split, axis=1)

    #group 4D by filename 

    grouped_tensor = tensor.groupby('filename')



    # Now, find matches from noblock and block
    def find_comparable(row):
        condition = (block['rows_per_block'] == row['rows']) & (block['cols_per_block'] == row['cols']) & (block['filename'] == row['filename'])
        matching_row = block.query("@condition")['dcsr_median'].values

        if len(matching_row) > 0:
            row['block_median'] = matching_row[0]
        else:
            row['block_median'] = None

        condition2 = (noblock['filename'] == row['filename'])
        matching_row2 = noblock.query('@condition2')['dcsr_median'].values
        if len(matching_row2) > 0:
            row['noblock_median'] = matching_row2[0]
        else:
            row['noblock_median'] = None
        return row

    # Save each combined dataframe in a file

    for filename, group_df in grouped_tensor:
        group_df = group_df.apply(find_comparable, axis=1)
        group_df = group_df.rename(columns={'dcsr_median':'tensor_median'})
        columns_to_save = ['filename', 'rows', 'cols', 'block_median', 'noblock_median','tensor_median']
        group_df[columns_to_save].to_csv(f'{filename}.csv', index=False)
    #clean 4D by removing all entries with NaN

if __name__ == '__main__':
    main()

