/*
This source file is part of the Global Atmospheric Modeling Framework (GAME), which is released under the MIT license.
Github repository: https://github.com/MHBalsmeier/game
*/

int set_grid_properties(Grid *, Dualgrid *, char[]);
int calc_delta_t(double, double *, Grid *);
int interpolation_t(State *, State *, State *, double, double, double);
int set_init_data(char[], State *);
int write_out(State *, double [], int, double, double, char[], Grid *);
int find_string_length_from_int(int, int *);
int find_hour_from_time_coord(double, int *, int *, int *, int *, int *, int *, int *);
int write_out_integral(State *, double, char [], Grid *, Dualgrid *, int);
