/*
This source file is part of the Geophysical Fluids Modeling Framework (GAME), which is released under the MIT license.
Github repository: https://github.com/AUN4GFD/game
*/

/*
In this file, spatial operators are collected which are only needed for the output.
*/

#include "../enum_and_typedefs.h"
#include "../spatial_operators/spatial_operators.h"
#include "../diagnostics/diagnostics.h"
#include <stdio.h>
#include <stdlib.h>

int inner_product_tangential(Vector_field, Vector_field, Scalar_field, Grid *, Dualgrid *);

int epv_diagnostics(Curl_field pot_vort, Scalar_field pot_temp, Scalar_field epv, Grid *grid, Dualgrid *dualgrid)
{
	// diagnozing Ertel's potential vorticity (EPV)
	// allocating memory for quantities we need in order to determine the EPV
	Vector_field *grad_pot_temp = calloc(1, sizeof(Vector_field));
	Vector_field *pot_vort_as_mod_vector_field = calloc(1, sizeof(Vector_field));
	int layer_index, h_index, scalar_index, j;
	double upper_weight, lower_weight, layer_thickness;
	#pragma omp parallel for private(layer_index, h_index, scalar_index, upper_weight, lower_weight, layer_thickness, j)
	for (int i = 0; i < NO_OF_VECTORS; ++i)
	{
		layer_index = i/NO_OF_VECTORS_PER_LAYER;
		h_index = i - layer_index*NO_OF_VECTORS_PER_LAYER;
		// diagnozing the horizontal vorticity at the primal horizontal vector points (they are TANGENTIAL! so it is not a real vector field, but a modified one)
		if (h_index >= NO_OF_SCALARS_H)
		{
			// determining the upper and lower weights
			layer_thickness =
			0.5*(grid -> z_vector[layer_index*NO_OF_VECTORS_PER_LAYER + grid -> from_index[h_index - NO_OF_SCALARS_H]]
			+ grid -> z_vector[layer_index*NO_OF_VECTORS_PER_LAYER + grid -> to_index[h_index - NO_OF_SCALARS_H]])
			- 0.5*(grid -> z_vector[(layer_index + 1)*NO_OF_VECTORS_PER_LAYER + grid -> from_index[h_index - NO_OF_SCALARS_H]]
			+ grid -> z_vector[(layer_index + 1)*NO_OF_VECTORS_PER_LAYER + grid -> to_index[h_index - NO_OF_SCALARS_H]]);
			if (layer_index == 0)
			{
				upper_weight =
				(0.5*(grid -> z_vector[layer_index*NO_OF_VECTORS_PER_LAYER + grid -> from_index[h_index - NO_OF_SCALARS_H]]
				+ grid -> z_vector[layer_index*NO_OF_VECTORS_PER_LAYER + grid -> to_index[h_index - NO_OF_SCALARS_H]])
				- grid -> z_vector[i])/layer_thickness;
			}
			else
			{
				upper_weight = 0.5*(grid -> z_vector[i - NO_OF_VECTORS_PER_LAYER] - grid -> z_vector[i])/layer_thickness;
			}
			if (layer_index == NO_OF_LAYERS - 1)
			{
				lower_weight = (grid -> z_vector[i]
				- 0.5*(grid -> z_vector[(layer_index + 1)*NO_OF_VECTORS_PER_LAYER + grid -> from_index[h_index - NO_OF_SCALARS_H]]
				+ grid -> z_vector[(layer_index + 1)*NO_OF_VECTORS_PER_LAYER + grid -> to_index[h_index - NO_OF_SCALARS_H]]))/layer_thickness;
			}
			else
			{
 				lower_weight = 0.5*(grid -> z_vector[i] - grid -> z_vector[i + NO_OF_VECTORS_PER_LAYER])/layer_thickness;
			}
			// determining the horizontal potential vorticity at the primal vector point
			(*pot_vort_as_mod_vector_field)[i] =
			upper_weight*pot_vort[layer_index*2*NO_OF_VECTORS_H + h_index - NO_OF_SCALARS_H]
			+ lower_weight*pot_vort[(layer_index + 1)*2*NO_OF_VECTORS_H + h_index - NO_OF_SCALARS_H];
		}
		// diagnozing the vertical component of the potential vorticity at the vertical vector points
		else
		{
			// initializing the value with zero
			(*pot_vort_as_mod_vector_field)[i] = 0;
			// highest layer
			if (layer_index == 0)
			{
				for (j = 0; j < 6; ++j)
				{
					scalar_index = h_index;
				    (*pot_vort_as_mod_vector_field)[i] +=
				    0.5*grid -> inner_product_weights[8*scalar_index + j]
				    *pot_vort[NO_OF_VECTORS_H + layer_index*2*NO_OF_VECTORS_H + grid -> adjacent_vector_indices_h[6*h_index + j]];
				}
			}
			// lowest layer
			else if (layer_index == NO_OF_LAYERS)
			{
				for (j = 0; j < 6; ++j)
				{
					scalar_index = (NO_OF_LAYERS - 1)*NO_OF_SCALARS_H + h_index;
				    (*pot_vort_as_mod_vector_field)[i] +=
				    0.5*grid -> inner_product_weights[8*scalar_index + j]
				    *pot_vort[NO_OF_VECTORS_H + (layer_index - 1)*2*NO_OF_VECTORS_H + grid -> adjacent_vector_indices_h[6*h_index + j]];
				}
			}
			// inner domain
			else
			{
				// contribution of upper cell
				for (j = 0; j < 6; ++j)
				{
					scalar_index = (layer_index - 1)*NO_OF_SCALARS_H + h_index;
				    (*pot_vort_as_mod_vector_field)[i] +=
				    0.25*grid -> inner_product_weights[8*scalar_index + j]
				    *pot_vort[NO_OF_VECTORS_H + (layer_index - 1)*2*NO_OF_VECTORS_H + grid -> adjacent_vector_indices_h[6*h_index + j]];
				}
				// contribution of lower cell
				for (j = 0; j < 6; ++j)
				{
					scalar_index = layer_index*NO_OF_SCALARS_H + h_index;
				    (*pot_vort_as_mod_vector_field)[i] +=
				    0.25*grid -> inner_product_weights[8*scalar_index + j]
				    *pot_vort[NO_OF_VECTORS_H + layer_index*2*NO_OF_VECTORS_H + grid -> adjacent_vector_indices_h[6*h_index + j]];
				}
			}
		}
	}
	// taking the gradient of the potential temperature
	grad(pot_temp, *grad_pot_temp, grid);
	inner_product_tangential(*pot_vort_as_mod_vector_field, *grad_pot_temp, epv, grid, dualgrid);
	// freeing the memory
	free(pot_vort_as_mod_vector_field);
	free(grad_pot_temp);
	return 0;
}

int inner_product_tangential(Vector_field in_field_0, Vector_field in_field_1, Scalar_field out_field, Grid *grid, Dualgrid *dualgrid)
{
    // This function computes the inner product of the two vector fields in_field_0 and in_field_1. This is needed for computing the dissipation due to momentum diffusion (friction).
    int layer_index, h_index, j;
    double tangential_wind_value;
    #pragma omp parallel for private (j, layer_index, h_index)
	for (int i = 0; i < NO_OF_SCALARS; ++i)
	{
	    layer_index = i/NO_OF_SCALARS_H;
	    h_index = i - layer_index*NO_OF_SCALARS_H;
	    out_field[i] = 0;
	    for (j = 0; j < 6; ++j)
	    {
			tangential_wind(in_field_1, layer_index, grid -> adjacent_vector_indices_h[6*h_index + j], &tangential_wind_value, grid);
	        out_field[i] +=
	        grid -> inner_product_weights[8*i + j]
	        *in_field_0[NO_OF_SCALARS_H + layer_index*NO_OF_VECTORS_PER_LAYER + grid -> adjacent_vector_indices_h[6*h_index + j]]
	        *tangential_wind_value;
	    }
	    out_field[i] += grid -> inner_product_weights[8*i + 6]*in_field_0[h_index + layer_index*NO_OF_VECTORS_PER_LAYER]*in_field_1[h_index + layer_index*NO_OF_VECTORS_PER_LAYER];
	    out_field[i] += grid -> inner_product_weights[8*i + 7]*in_field_0[h_index + (layer_index + 1)*NO_OF_VECTORS_PER_LAYER]*in_field_1[h_index + (layer_index + 1)*NO_OF_VECTORS_PER_LAYER];
	}
    return 0;
}

int interpolate_to_ll(double in_field[], double out_field[], Grid *grid)
{
	/*
	This function interpolates a scalar field to a lat-lon grid.
	*/
	int i, j;
	#pragma omp parallel for private(i, j)
	for (i = 0; i < NO_OF_LATLON_IO_POINTS; ++i)
	{
		out_field[i] = 0;
		for (j = 0; j < 3; ++j)
		{
			out_field[i] += grid -> latlon_interpol_weights[3*i + j]*in_field[grid -> latlon_interpol_indices[3*i + j]];
		}
	}
	return 0;
}







