/*
This source file is part of the Geophysical Fluids Modeling Framework (GAME), which is released under the MIT license.
Github repository: https://github.com/MHBalsmeier/game
*/

#include "../enum_and_typedefs.h"
#include "spatial_operators.h"
#include "../diagnostics/diagnostics.h"

int momentum_diff_diss(State *state, Diagnostics *diagnostics, Irreversible_quantities *diffusion, Config_info *config_info, Grid *grid, Dualgrid *dualgrid)
{	
	// Firstly the diffusion.
	// Evaluating necessary differential operators.
	divv_h(state -> velocity_gas, diagnostics -> velocity_gas_divv, grid);
	add_vertical_divv(state -> velocity_gas, diagnostics -> velocity_gas_divv, grid);
    grad(diagnostics -> velocity_gas_divv, diffusion -> friction_acc, grid);
    curl_of_vorticity_m(diagnostics -> rel_vort, diagnostics -> curl_of_vorticity_m, grid, dualgrid);
    // Calculating the effective viscosity coefficients.
    calc_divv_term_viscosity_eff(state, config_info, diffusion -> divv_term_viscosity_eff);
    calc_curl_term_viscosity_eff(state, config_info, diffusion -> curl_term_viscosity_eff);
    // Multiplying the values of the differential operators by the effective viscosities.
	scalar_times_vector(diffusion -> divv_term_viscosity_eff, diffusion -> friction_acc, diffusion -> friction_acc, grid, 0);
	scalar_times_vector(diffusion -> curl_term_viscosity_eff, diagnostics -> curl_of_vorticity_m, diffusion -> friction_acc, grid, 1);
	// dissipation remains to be implemented
	return 0;
}

int curl_of_vorticity_m(Curl_field vorticity, Vector_field out_field, Grid *grid, Dualgrid *dualgrid)
{
	int layer_index, h_index, no_of_edges, sign;
	#pragma omp parallel for private(layer_index, h_index, no_of_edges, sign)
	for (int i = 0; i < NO_OF_VECTORS; ++i)
	{
		layer_index = i/NO_OF_VECTORS_PER_LAYER;
		h_index = i - layer_index*NO_OF_VECTORS_PER_LAYER;
		// The vertical case.
		if (h_index < NO_OF_SCALARS_H)
		{
			out_field[i] = 0;
			no_of_edges = 6;
			if (h_index < NO_OF_PENTAGONS)
			{
				no_of_edges = 5;
			}
			for (int j = 0; j < no_of_edges; ++j)
			{
				// The sign indicates the positive curl direction.
				sign = -dualgrid -> h_curl_signs[4*grid -> adjacent_vector_indices_h[6*h_index + j]]*grid -> adjacent_signs_h[6*h_index + j];
				out_field[i] += sign
				*dualgrid -> normal_distance[layer_index*NO_OF_DUAL_VECTORS_PER_LAYER + grid -> adjacent_vector_indices_h[6*h_index + j]]
				*vorticity[layer_index*2*NO_OF_VECTORS_H + grid -> adjacent_vector_indices_h[6*h_index + j]];
			}
			// Dividing by the area. The additional minus sign accounts for the fact the we want the negative curl of the curl.
			out_field[i] = -out_field[i]/grid -> area[i];
		}
		// The horizontal case.
		// Remember: (curl(zeta))*e_x = dzeta_z/dy - dzeta_y/dz = (dz*dzeta_z - dy*dzeta_y)/(dy*dz) = (dz*dzeta_z - dy*dzeta_y)/area (Stokes' Theorem, which is used here)
		else
		{
			out_field[i] = 0;
			// horizontal difference of vertical vorticity (dzeta_z/dy)
			// An averaging over three rhombi must be done.
			for (int j = 0; j < 3; ++j)
			{
				out_field[i] +=
				-dualgrid -> h_curl_signs[4*(h_index - NO_OF_SCALARS_H)]
				// This prefactor accounts for the fact that we average over three rhombi.
				*1.0/3*(
				// vertical length at the to_index_dual point
				dualgrid -> normal_distance[NO_OF_DUAL_SCALARS_H + layer_index*NO_OF_DUAL_VECTORS_PER_LAYER + dualgrid -> to_index[h_index - NO_OF_SCALARS_H]]
				// vorticity at the to_index_dual point
				*vorticity[NO_OF_VECTORS_H + layer_index*2*NO_OF_VECTORS_H + dualgrid -> adjacent_vector_indices_h[3*dualgrid -> to_index[h_index - NO_OF_SCALARS_H] + j]]
				// vertical length at the from_index_dual point
				- dualgrid -> normal_distance[NO_OF_DUAL_SCALARS_H + layer_index*NO_OF_DUAL_VECTORS_PER_LAYER + dualgrid -> from_index[h_index - NO_OF_SCALARS_H]]
				// vorticity at the from_index_dual point
				*vorticity[NO_OF_VECTORS_H + layer_index*2*NO_OF_VECTORS_H + dualgrid -> adjacent_vector_indices_h[3*dualgrid -> from_index[h_index - NO_OF_SCALARS_H] + j]]);
			}
			// vertical difference of horizontal vorticity (-dzeta_y/dz)
			out_field[i] +=
			// substracting the upper zeta_y value
			dualgrid -> h_curl_signs[4*(h_index - NO_OF_SCALARS_H)]
			*dualgrid -> normal_distance[layer_index*NO_OF_DUAL_VECTORS_PER_LAYER + h_index - NO_OF_SCALARS_H]
			*vorticity[layer_index*2*NO_OF_VECTORS_H + h_index - NO_OF_SCALARS_H];
			out_field[i] +=
			// adding the lower zeta_y value
			-dualgrid -> h_curl_signs[4*(h_index - NO_OF_SCALARS_H)]
			*dualgrid -> normal_distance[(layer_index + 1)*NO_OF_DUAL_VECTORS_PER_LAYER + h_index - NO_OF_SCALARS_H]
			*vorticity[(layer_index + 1)*2*NO_OF_VECTORS_H + h_index - NO_OF_SCALARS_H];
			// Dividing by the area. The additional minus sign accounts for the fact the we want the negative curl of the curl.
			out_field[i] = -out_field[i]/grid -> area[i];
		}
	}
	return 0;
}










