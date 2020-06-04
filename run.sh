#!/bin/bash
time_string=$(date --utc +%Y%m%d%H%M%S)
if [ $overwrite_run_id -eq 0 ]
then
run_id="$operator"_"$init_state_filename"_"$time_string"
fi
output_dir=$output_dir_base/$run_id
if [ -d $output_dir ]
then
rm -r $output_dir
fi
mkdir $output_dir
./core/game $run_span $write_out_interval $grid_props_file $init_state_file $output_dir $cfl_margin $dissipation $rad_on $add_comps_on $operator $write_out_dry_mass_integral $write_out_entropy_integral $write_out_energy_integral