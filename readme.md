# High Level Optimizations for Sparse Computations

## Description
This project aims to explore different high level optimizations for sparse computations, more specifically Sparse Matrix Vector Multiplication (SpMV).

## How to Use

### Installation
1. **Install Conda:** If you haven't already, install Conda by following the instructions provided in the respective documentation page [here](https://docs.anaconda.com/free/miniconda/).

For Windows Terminal, add the miniconda binary path to the `Path` environment variable. Then open PowerShell as admin and execute:
```
  Set-ExecutionPolicy -Scope CurrentUser -ExecutionPolicy Restricted
  conda init powershell
```

2. **Create Conda Environment:** 
   - Clone this repository.
   - Navigate to the root directory of this repository.
   - Run the following command in your terminal to create a Conda environment using the provided `environment.yml` file:
     ```
     conda env create --name sparse-high --file=environment.yml
     ```
    For Windows, use the Windows environment file:
     ```
     conda env create --name sparse-high --file=environment.win.yml
     ```

    

### Usage
1. **Activate Environment:** Before running any scripts or experiments, activate the Conda environment you created. You can do this by running:
    ```
    conda activate sparse-high

    ```

