#!/bin/bash
#SBATCH --account=k1285
#SBATCH --job-name=10240-64-1
#SBATCH --output=/project/k1285/CombBLAS/run/lubm10240/lubm10240_nodes64-cores1024-1.log
#SBATCH --error=/project/k1285/CombBLAS/run/lubm10240/lubm10240_nodes64-cores1024-1.err
#SBATCH --nodes=64
#SBATCH --time=01:00:00
#SBATCH --hint=nomultithread
#SBATCH --ntasks-per-node=16
#SBATCH --ntasks-per-socket=8
#SBATCH --ntasks=1024
#SBATCH --exclusive

srun /project/k1285/CombBLAS/build/selfTests/lubm10240
