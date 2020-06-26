#!/bin/bash
operator=MHB
overwrite_run_id=1
run_id=jw_balanced_dry_reversible
run_span=3600
write_out_interval=900
grid_props_file=/home/max/compiled/game/grids/B4L26T30000_M2_O2_OL17.nc
init_state_filename=test_2_B4L26T30000_M2_O2_OL17.grb2
init_state_file=/home/max/compiled/game/input/$init_state_filename
output_dir_base=/home/max/compiled/game/output
cfl_margin=0.2
diffusion_on=0
dissipation_on=0
tracers_on=0
rad_on=0
radiation_delta_t=3600
tracers_dynamics_delta_t_ratio=3
write_out_mass_dry_integral=1
write_out_entropy_gas_integral=1
write_out_energy_integral=1
source run.sh
