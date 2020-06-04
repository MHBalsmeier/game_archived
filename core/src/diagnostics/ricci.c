#include "../enum_and_typedefs.h"
#include <math.h>

double inner_elementary(double [], double []);

int vertical_contravariant_normalized(Vector_field in_field, int layer_index, int h_index, Grid *grid, double *result)
{
	*result = in_field[layer_index*NUMBER_OF_VECTORS_PER_LAYER + h_index];
	return 0;
}

int horizontal_covariant_normalized(Vector_field in_field, int layer_index, int h_index, Grid *grid, double *result)
{
	*result = in_field[layer_index*NUMBER_OF_VECTORS_PER_LAYER + NUMBER_OF_VECTORS_V + h_index];
	return 0;
}

double inner_elementary(double vector_0[], double vector_1[])
{
	double result = 0;
	for (int i = 0; i < 3; ++i)
		result += vector_0[i]*vector_1[i];
	return result;
}