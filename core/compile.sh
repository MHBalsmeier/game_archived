LD_LIBRARY_PATH=/home/max/source/eccodes/build/lib/
export LD_LIBRARY_PATH
gcc src/coordinator.c src/io/set_grid_props_and_dt.c src/io/init_state_setter.c src/io/write_output.c src/time_stepping/euler_explicit.c src/io/interpolation_t.c src/r_operators/rot.c src/r_operators/scalar_product.c src/r_operators/scalar_times_vector.c src/r_operators/determine_tendency.c src/r_operators/vector_product.c src/r_operators/div.c src/r_operators/laplace.c src/r_operators/grad.c src/r_operators/laplace_vec.c src/r_operators/rot_dual.c src/diagnostics/exner_pressure_diagnostics.c src/r_operators/recover_components/recov_hor_par_dual.c src/r_operators/recover_components/recov_hor_par_pri.c src/r_operators/recover_components/recov_hor_ver_dual.c src/r_operators/recover_components/recov_hor_ver_pri.c src/r_operators/recover_components/recov_ver_1_dual.c src/r_operators/recover_components/recov_ver_1_pri.c src/r_operators/recover_components/recov_ver_2_dual.c src/r_operators/recover_components/recov_ver_2_pri.c /home/max/source/eccodes/build/lib/libeccodes.so -l netcdf -lm -o game
