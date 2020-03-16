int grad(Scalar_field, Vector_field, Grid *);
int laplace(Scalar_field, Scalar_field, Grid *);
int laplace_vec(Vector_field, Vector_field, Grid *, Dualgrid *);
int rot(Vector_field, Dual_vector_field, Grid *, Dualgrid *);
int rot_dual(Dual_vector_field, Vector_field, Grid *, Dualgrid *);
int vector_product(Vector_field, Dual_vector_field, Vector_field, Grid *);
int scalar_product(Vector_field, Vector_field,Scalar_field, Grid *);
int divergence(Vector_field, Scalar_field, Grid *);
int recov_hor_par_dual(Dual_vector_field, double[], Grid *);
int recov_hor_par_pri(Vector_field, double out_field[], Grid *);
int recov_hor_ver_dual(Dual_vector_field, double[], Grid *);
int recov_hor_ver_pri(Vector_field, double[], Grid *);
int recov_ver_1_dual(Dual_vector_field, double[], Grid *);
int recov_ver_1_pri(Vector_field, double[], Grid *);
int recov_ver_2_dual(Dual_vector_field, double[], Grid *);
int recov_ver_2_pri(Vector_field, double[], Grid *);
int tendency(State *, State *, Grid *, Dualgrid*);
int scalar_times_vector(Scalar_field, Vector_field, Vector_field, Grid *);
