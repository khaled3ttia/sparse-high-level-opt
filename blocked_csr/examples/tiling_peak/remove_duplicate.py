import os
import pandas as pd

def process_csv(file_path):
    # Read the entire file as a list of lines
    with open(file_path, 'r') as file:
        lines = file.readlines()
    
    # Identify the last occurrence of the header
    header = lines[0].strip()
    header_indexes = [i for i, line in enumerate(lines) if line.strip() == header]
    last_header_index = header_indexes[-1] if header_indexes else 0
    
    # Keep only the last part after the last header
    lines_to_keep = lines[last_header_index:]
    
    # Write the filtered lines back to the file
    with open(file_path, 'w') as file:
        file.writelines(lines_to_keep)

def process_directory(directory_path):
    for filename in os.listdir(directory_path):
        if filename.endswith(".csv"):
            file_path = os.path.join(directory_path, filename)
            process_csv(file_path)

# Example usage
directory_path = "snap_trial2"  # replace with your directory path
process_directory(directory_path)

