import subprocess
import argparse

def run(files):
    for file in files:
        cmd = f'sbatch {file}'
        out = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, cwd='.', shell=True)
        print(f'sbatch {file} submitted...')
        if out.returncode != 0 :
            print(f"Failed with error {out.stderr.decode('utf-8')}")

def main(args):
    ngroups = args.ngroups

    sbatch_config = { 
                'partition' : 'disc',
                'nodes' : 1, 
                'ntasks' : 128,
                'mem' : '256G',
                'time' : '48:00:00',
                'mail-user' : 'khaled.abdelaal@ou.edu',
                'mail-type' : 'ALL',
                'chdir' : '/home/khaled'
            }
    files = []
    for gid in range(1,ngroups+1):

        sbatch_config['output'] = f'taco_blocking_group{gid}_%J_stdout.txt'
        sbatch_config['error'] = f'taco_blocking_group{gid}_%J_stderr.txt'
        sbatch_config['job-name'] = f'taco_block_group{gid}'
       
        lines = ['#!/bin/bash','#','#SBATCH --exclusive'] 
        
        
        for k,v in sbatch_config.items():
            lines.append(f'#SBATCH --{k}={v}')
       
        lines.append(f"{'#'*30}")
        print(lines)

        current_in_out_dir = f'{args.base}{gid}'
    
        lines.append(f'cp {args.script} {current_in_out_dir}')
        
        lines.append('echo "Starting Apptainer Container..."')
        lines.append(f'apptainer exec --bind {current_in_out_dir}:/data taco_py_latest.sif /data/run_tool_blocking.sh /data/ /data/taco_block_group{gid}.csv 10')

        lines.append('echo "CPU SPEC"')
        lines.append('lscpu')
        
        files.append(f'taco_block_group{gid}.sbatch')
        with open(files[gid-1],'w') as f:
            for line in lines:
                f.write(f'{line}\n')
    run(files)


if __name__ == '__main__':
    
    parser = argparse.ArgumentParser()
    parser.add_argument('--ngroups', type=int, default=6, help='Number of Input Matrix Directories (groups)')
    parser.add_argument('--base', type=str, default='/scratch/khaled/mtx_group', help='Base Input Matrix Directory path excluding group number (on cluster)')
    parser.add_argument('--script', type=str, default='/home/khaled/sparse-high-level-opt/taco_expr/block/run_tool_blocking.sh', help='Bash script path (script to be running within Apptainer on cluster)')
    parser.add_argument('--image', type=str, default='taco_py_latest.sif', help='Appatiner image path (on cluster)')
    parser.add_argument('--outdir', type=str, default='.', help='Path to generate output sbatch files')
    args = parser.parse_args()
    main(args)
