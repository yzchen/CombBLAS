#!/bin/bash
#SBATCH --account=k1285
#SBATCH --job-name=100k-2k-1
#SBATCH --output=/project/k1285/CombBLAS/run/lubm100k/lubm100k_nodes2k-cores64k-1.log
#SBATCH --error=/project/k1285/CombBLAS/run/lubm100k/lubm100k_nodes2k-cores64k-1.err
#SBATCH --nodes=2048
#SBATCH --time=01:00:00
#SBATCH --hint=nomultithread
#SBATCH --ntasks-per-node=32
#SBATCH --ntasks-per-socket=16
#SBATCH --ntasks=65536
#SBATCH --exclusive

srun /project/k1285/CombBLAS/build/selfTests/lubm100k
