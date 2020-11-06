/*
This source file is part of the Geophysical Fluids Modeling Framework (GAME), which is released under the MIT license.
Github repository: https://github.com/MHBalsmeier/game
*/
/*
In this source file, the calculation of the explicit part of the momentum equation is managed.
*/

#include <stdlib.h>
#include <stdio.h>
#include "../../enum_and_typedefs.h"
#include "../../spatial_operators/spatial_operators.h"
#include "atmostracers.h"
#include "../../diagnostics/diagnostics.h"

int integrate_momentum(State *state, State *state_tendency, Grid *grid, Dualgrid *dualgrid, Diagnostics *diagnostics, Forcings *forcings, Diffusion_info *diffusion_info, Config_info *config_info, int no_step_rk)
{
	// Here, the gaseous flux density is prepared for the generalized Coriolis term.
	#pragma omp parallel for
	for (int i = 0; i < NO_OF_SCALARS; ++i)
	{
		diagnostics -> density_gen[i] = density_gas(state, i);
	}
    scalar_times_vector(diagnostics -> density_gen, state -> velocity_gas, diagnostics -> flux_density, grid, 0);
    // Now, the potential vorticity is evaluated.
    calc_pot_vort(state -> velocity_gas, diagnostics -> density_gen, diagnostics, grid, dualgrid);
    // Now, the generalized Coriolis term is evaluated.
    coriolis_gen(diagnostics -> flux_density, diagnostics -> pot_vort, forcings -> pot_vort_tend, grid);
    // Horizontal kinetic energy is prepared for the gradient term of the Lamb transformation.
    kinetic_energy(state -> velocity_gas, diagnostics -> e_kin_h, grid, 0);
    grad(diagnostics -> e_kin_h, forcings -> e_kin_h_grad, grid);
    // Now the explicit forces are added up.
    int layer_index, h_index;
    double metric_term, vertical_velocity, hor_non_trad_cori_term, r_h, c_h_p;
    #pragma omp parallel for private(layer_index, h_index, metric_term, vertical_velocity, hor_non_trad_cori_term,  r_h, c_h_p)
    for (int i = 0; i < NO_OF_VECTORS; ++i)
    {
    	layer_index = i/NO_OF_VECTORS_PER_LAYER;
    	h_index = i - layer_index*NO_OF_VECTORS_PER_LAYER;
        if (i < NO_OF_SCALARS_H || i >= NO_OF_VECTORS - NO_OF_SCALARS_H)
            state_tendency -> velocity_gas[i] = 0;
        else
        {
    		if (h_index >= NO_OF_SCALARS_H)
    		{
    			recov_hor_ver_pri(state -> velocity_gas, layer_index, h_index - NO_OF_SCALARS_H, &vertical_velocity, grid);
    			metric_term = -vertical_velocity*state -> velocity_gas[i]/(RADIUS + grid -> z_vector[i]);
    			hor_non_trad_cori_term = -vertical_velocity*dualgrid -> f_vec[2*NO_OF_VECTORS_H + h_index - NO_OF_SCALARS_H];
        		state_tendency -> velocity_gas[i] = forcings -> pressure_gradient_acc[i] + forcings -> pot_vort_tend[i] - grid -> gravity_m[i] - forcings -> e_kin_h_grad[i] + hor_non_trad_cori_term + metric_term + diffusion_info -> friction_acc[i];
    		}
        	if (h_index < NO_OF_SCALARS_H)
        	{
				if (layer_index == 0)
				{
					r_h = gas_constant_diagnostics(state, h_index);
					c_h_p = spec_heat_cap_diagnostics_p(state, h_index);
				}
				else if (layer_index == NO_OF_LAYERS)
				{
					r_h = gas_constant_diagnostics(state, (layer_index - 1)*NO_OF_SCALARS_H + h_index);
					c_h_p = spec_heat_cap_diagnostics_p(state, (layer_index - 1)*NO_OF_SCALARS_H + h_index);
				}
				else
				{
					r_h = 0.5*(gas_constant_diagnostics(state, (layer_index - 1)*NO_OF_SCALARS_H + h_index)
					+ gas_constant_diagnostics(state, layer_index*NO_OF_SCALARS_H + h_index));
					c_h_p = 0.5*(spec_heat_cap_diagnostics_p(state, (layer_index - 1)*NO_OF_SCALARS_H + h_index)
					+ spec_heat_cap_diagnostics_p(state, layer_index*NO_OF_SCALARS_H + h_index));
				}
        		state_tendency -> velocity_gas[i] = forcings -> pot_vort_tend[i] - forcings -> e_kin_h_grad[i] - grid -> gravity_m[i] + r_h/c_h_p*forcings -> pressure_gradient_acc[i] + diffusion_info -> friction_acc[i];
    		}
        }
    }
    return 0;
}
    
    
    
    
    
    
    
    
    
    
    