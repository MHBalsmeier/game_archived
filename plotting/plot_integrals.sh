#!/bin/bash

# This source file is part of the Geophysical Fluids Modeling Framework (GAME), which is released under the MIT license.
# Github repository: https://github.com/MHBalsmeier/game

game_home_dir=/home/max/compiled/game_dev # the home directory of GAME
run_id=ullrich_dry_irrev # the run id which you want to plot
output_dir=$game_home_dir/output/$run_id # the directory where the grib files are stored
fig_save_path=/home/max/figs/game_output/$run_id # the directory to which the figures will be saved
plot_mass_dry_integral=1 # set this to one if you want to plot the dry mass of the atmospheric domain
plot_entropy_gas_integral=1 # set this to one if you want to plot the entropy of the atmospheric domain
plot_energy_integral=1 # set this to one if you want to plot the energy forms of the atmospheric domain
plot_linearized_entropy_gas_integral=1 # set this to one if you want to plot the "linearized entropy" (c_p*density*log(potential temperature)) of the atmospheric domain
source $game_home_dir/plotting/.sh/integrals_root.sh # this is the script from which the python plot scripts are called