/*
This source file is part of the Geophysical Fluids Modeling Framework (GAME), which is released under the MIT license.
Github repository: https://github.com/MHBalsmeier/game
*/

#include "../enum_and_typedefs.h"
#include "../diagnostics/diagnostics.h"
#include <stdlib.h>
#include <stdio.h>

int vorticity_flux(Vector_field mass_flux_density, Curl_field pot_vorticity, Vector_field out_field, Grid *grid, Dualgrid *dualgrid)
{
    int layer_index, h_index;
    double upper_weight, lower_weight, upper_value, lower_value, z_lower, z_upper;
	#pragma omp parallel for private(layer_index, h_index,  upper_weight, lower_weight, upper_value, lower_value, z_lower, z_upper)
    for (int i = 0; i < NO_OF_VECTORS; ++i)
    {
        layer_index = i/NO_OF_VECTORS_PER_LAYER;
        h_index = i - layer_index*NO_OF_VECTORS_PER_LAYER;
        if (h_index >= NO_OF_SCALARS_H)
        {
        	// vertical potential vorticity times horizontal mass flux density
            vorticity_flux_horizontal_traditional(mass_flux_density, pot_vorticity, layer_index, h_index - NO_OF_SCALARS_H, &out_field[i], grid);
            // horizontal potential vorticity times horizontal mass flux density
            // z posiiton of the upper end of the face
            z_upper = 0.5*(grid -> z_vector[layer_index*NO_OF_VECTORS_PER_LAYER + grid -> from_index[h_index - NO_OF_SCALARS_H]]
            + grid -> z_vector[layer_index*NO_OF_VECTORS_PER_LAYER + grid -> to_index[h_index - NO_OF_SCALARS_H]]);
            // z posiiton of the upper end of the face
            z_lower = 0.5*(grid -> z_vector[(layer_index + 1)*NO_OF_VECTORS_PER_LAYER + grid -> from_index[h_index - NO_OF_SCALARS_H]]
            + grid -> z_vector[(layer_index + 1)*NO_OF_VECTORS_PER_LAYER + grid -> to_index[h_index - NO_OF_SCALARS_H]]);
            // vorticity flux value at upper end of the face
            upper_value = 0.5*(mass_flux_density[layer_index*NO_OF_VECTORS_PER_LAYER + grid -> from_index[h_index - NO_OF_SCALARS_H]]
            + mass_flux_density[layer_index*NO_OF_VECTORS_PER_LAYER + grid -> to_index[h_index - NO_OF_SCALARS_H]])
            *pot_vorticity[h_index - NO_OF_SCALARS_H + layer_index*2*NO_OF_VECTORS_H];
            // vorticity flux value at lower end of the face
            lower_value = 0.5*(mass_flux_density[(layer_index + 1)*NO_OF_VECTORS_PER_LAYER + grid -> from_index[h_index - NO_OF_SCALARS_H]]
            + mass_flux_density[(layer_index + 1)*NO_OF_VECTORS_PER_LAYER + grid -> to_index[h_index - NO_OF_SCALARS_H]])
            *pot_vorticity[h_index - NO_OF_SCALARS_H  + (layer_index + 1)*2*NO_OF_VECTORS_H];
            // determining the weights
            upper_weight = (z_upper - grid -> z_vector[i])/(z_upper - z_lower);
            lower_weight = (grid -> z_vector[i] - z_lower)/(z_upper - z_lower);
            // adding to the result
            out_field[i] += -(upper_weight*upper_value + lower_weight*lower_value);
        }
        else
        {    
			vorticity_flux_vertical(mass_flux_density, pot_vorticity, layer_index, h_index, &out_field[i], grid, dualgrid);
        }
    }
    return 0;
}