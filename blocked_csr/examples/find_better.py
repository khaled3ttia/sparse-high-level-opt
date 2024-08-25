import os
import pandas as pd
import argparse

def process_csv(file_path):
    # Read the CSV file into a DataFrame
    df = pd.read_csv(file_path)
    
    # Find the median execution time for no tiling configuration (rows=0, cols=0)
    no_tiling_median = df[(df['rows'] == 0) & (df['cols'] == 0)]['median'].values[0]
    
    # Total number of configurations
    total_config_count = len(df) - 1 
    
    # Find configurations that perform better than no tiling
    better_configs = df[df['median'] < no_tiling_median]
    
    # Sort the configurations by median execution time
    better_configs = better_configs.sort_values(by='median')
    
    # Count the number of better configurations
    better_config_count = len(better_configs)
    
    # Calculate the percentage of better configurations
    better_config_percentage = 0 if total_config_count == 0 else (better_config_count / total_config_count) * 100
    
    return better_configs, total_config_count, better_config_count, better_config_percentage

def main(input_dir, output_dir, summary_file):
    # Ensure output directory exists
    os.makedirs(output_dir, exist_ok=True)
    
    summary_data = []
    
    # Process each CSV file in the input directory
    for file_name in os.listdir(input_dir):
        if file_name.endswith('.csv'):
            input_file_path = os.path.join(input_dir, file_name)
            better_configs, total_config_count, better_config_count, better_config_percentage = process_csv(input_file_path)
            
            if not better_configs.empty:
                # Output the better configurations to a new CSV file
                output_file_name = f"better_configs_{file_name}"
                output_file_path = os.path.join(output_dir, output_file_name)
                better_configs.to_csv(output_file_path, index=False)
                
                # Add entry to summary data
                summary_data.append({
                    'file_name': file_name,
                    'total_config_count': total_config_count,
                    'better_config_count': better_config_count,
                    'better_config_percentage': better_config_percentage
                })
            
                print(f"Processed {file_name} and saved results to {output_file_name}")
    
    # Save the summary data to a CSV file
    summary_df = pd.DataFrame(summary_data)
    summary_df.to_csv(summary_file, index=False)
    print(f"Summary saved to {summary_file}")

if __name__ == "__main__":
    # Set up argparse to handle command-line arguments
    parser = argparse.ArgumentParser(description="Process CSV files to find better SpMV configurations.")
    parser.add_argument("--i", type=str, help="Directory containing input CSV files.")
    parser.add_argument("--o", type=str, help="Directory to save output CSV files.")
    parser.add_argument("--s", type=str, help="Path to save the summary CSV file.")
    
    # Parse the command-line arguments
    args = parser.parse_args()
    
    # Run the main function with the provided arguments
    main(args.i, args.o, args.s)
