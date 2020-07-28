/*
This source file is part of the Global Atmospheric Modeling Framework (GAME), which is released under the MIT license.
Github repository: https://github.com/MHBalsmeier/game
*/

int manage_time_stepping(State *, State *, Interpolate_info *, Grid *, Dualgrid *, Scalar_field, State *, Diagnostics *, Forcings *, Diffusion_info *, Config_info *, double);
int setup_interpolation(State *, Interpolate_info *);
int update_interpolation(State *, State *, Interpolate_info *);
int set_interpolated_temperature(State *, State *, Interpolate_info *, int);

