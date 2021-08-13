#!/bin/bash

# This source file is part of the Geophysical Fluids Modeling Framework (GAME), which is released under the MIT license.
# Github repository: https://github.com/OpenNWP/GAME

# test_ids:
# 0:	standard atmosphere with oro_id = 0
# 1:	standard atmosphere with oro_id = 1
# 2:	standard atmosphere with oro_id = 2
# 3:	Ullrich test, dry, perturbed
# 4:	Ullrich test, moist, perturbed
# 5:	Ullrich test, dry, perturbed, with oro_id = 3
# 6:	Ullrich test, moist, perturbed, with oro_id = 3
# 7:	Ullrich test, dry, with Gaussian mountain

test_id=3
type_of_vertical_grid=0 # 0: terrain following coordinates, 1: block-like orography
orography_layers=23 # number of layers following orography (only relevant if type_of_vertical_grid == 0)
toa=41152 # height of the top of the atmosphere
valgrind_check=0 # set this to 1, if you want to check the code with Valgrind
number_of_cpus=1 # relevant only for MPI
export OMP_NUM_THREADS=1 # relevant only for OMP
source .sh/run.sh
