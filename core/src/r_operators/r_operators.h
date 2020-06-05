int grad(Scalar_field, Vector_field, Grid *);
int adv_scalar(Scalar_field, Vector_field, Scalar_field, Grid *, Dualgrid *);
int curl(Vector_field, Dual_vector_field, Grid *, Dualgrid *);
int cross_product(Vector_field, Dual_vector_field, Vector_field, Grid *);
int inner(Vector_field, Vector_field, Scalar_field, Grid *, Dualgrid *);
int divergence(Vector_field, Scalar_field, Grid *, int);
int tendency(State *, State *, Grid *, Dualgrid*, int, int, int, double);
int scalar_times_scalar(Scalar_field, Scalar_field, Scalar_field, Grid *);
int scalar_times_vector(Scalar_field, Vector_field, Vector_field, Grid *);
int scalar_times_vector_h_v(Scalar_field, Scalar_field, Vector_field, Vector_field, Grid *);
int linear_combine_two_states(State *, State *, State *, double, double);
int add_vector_fields(Vector_field, Vector_field, Vector_field);
int dissipation(Vector_field, Scalar_field, Vector_field, Scalar_field, Grid *);
