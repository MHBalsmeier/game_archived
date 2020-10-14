/*
This source file is part of the Geophysical Fluids Modeling Framework (GAME), which is released under the MIT license.
Github repository: https://github.com/MHBalsmeier/game
*/

int grad(Scalar_field, Vector_field, Grid *);
int scalar_times_grad(Scalar_field, Scalar_field, Vector_field, Grid *);
int calc_pot_vort(Vector_field, Scalar_field, Diagnostics *, Grid *, Dualgrid *);
int add_f_to_rel_vort(Curl_field, Curl_field, Dualgrid *);
int calc_rel_vort(Vector_field, Curl_field, Grid *, Dualgrid *);
int coriolis_gen(Vector_field, Dual_vector_field, Vector_field, Grid *);
int kinetic_energy(Vector_field, Scalar_field, Grid *, int);
int divv_h(Vector_field, Scalar_field, Grid *);
int add_vertical_divv(Vector_field, Scalar_field, Grid *);
int scalar_times_scalar(Scalar_field, Scalar_field, Scalar_field);
int scalar_times_vector(Scalar_field, Vector_field, Vector_field, Grid *, int);
int scalar_times_vector_scalar_h_v(Scalar_field, Scalar_field, Vector_field, Vector_field, Grid *);
int scalar_times_vector_vector_h_v(Scalar_field, Vector_field, Vector_field, Vector_field, Grid *);
int linear_combine_two_states(State *, State *, State *, double, double);
int momentum_diff_diss(State *, Diagnostics*, Diffusion_info*, Config_info *, Grid *, Dualgrid *);
int grad_v_scalar_column(double [], double [], int, Grid *);
int inner_product(Vector_field, Vector_field, Scalar_field, Grid *);
int curl_of_vorticity_m(Curl_field, Vector_field, Grid *, Dualgrid *);
