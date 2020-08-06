/*
This source file is part of the Global Atmospheric Modeling Framework (GAME), which is released under the MIT license.
Github repository: https://github.com/MHBalsmeier/game
The vertical advection of horizontal momentum is organized here.
*/

#include "../enum_and_typedefs.h"
#include <stdlib.h>
#include <stdio.h>
#include "../diagnostics/diagnostics.h"
#include <omp.h>
#include "../spatial_operators/spatial_operators.h"

int manage_pressure_gradient(State *current_state, Grid *grid, Dualgrid *dualgrid, Diagnostics *diagnostics, Forcings *forcings, Interpolate_info *interpolation, Diffusion_info *diffusion_info, Config_info *config_info, int no_step_rk)
{
	double old_hor_grad_weight = -C_D_V/C_D_P;
	double new_hor_grad_weight = 1 - old_hor_grad_weight;
	if (config_info -> totally_first_step_bool == 1)
	{
		old_hor_grad_weight = 0;
		new_hor_grad_weight = 1;
	}
    double rho_h, pressure_value_d, pressure_value_v, pressure_value, pot_temp, pot_temp_v;
    #pragma omp parallel for private(rho_h, pressure_value_d, pressure_value_v, pressure_value, pot_temp, pot_temp_v)
    for (int i = 0; i < NO_OF_SCALARS; ++i)
    {
		rho_h = current_state -> density_dry[i] + current_state -> tracer_densities[NO_OF_CONDENSATED_TRACERS*NO_OF_SCALARS + i];
    	pressure_value_d = current_state -> density_dry[i]*R_D*current_state -> temp_gas[i];
    	pressure_value_v = current_state -> tracer_densities[2*NO_OF_SCALARS + i]*R_V*current_state -> temp_gas[i];
    	pressure_value = pressure_value_d + pressure_value_v; 
    	// Preparation of dry part of the second pressure gradient term.
    	pot_temp = current_state -> temp_gas[i]*pow(P_0/pressure_value, R_D/C_D_P);
		diagnostics -> specific_entropy_dry[i] = C_D_P*(log(pot_temp) + ENTROPY_CONSTANT_D);
		diagnostics -> pressure_gradient_1_dry_prefactor[i] = current_state -> temp_gas[i]*current_state -> density_dry[i]/rho_h;
    	// Preparation of water vapour part of the second pressure gradient term.
    	pot_temp_v = current_state -> temp_gas[i]*pow(P_0/pressure_value, R_V/C_V_P);
		diagnostics -> specific_entropy_vapour[i] = C_V_P*(log(pot_temp_v) + ENTROPY_CONSTANT_V);
		diagnostics -> pressure_gradient_1_vapour_prefactor[i] = current_state -> temp_gas[i]*current_state -> tracer_densities[2*NO_OF_SCALARS + i]/rho_h;
	}
	// Before the calculation of the new pressure gradient, the old value needs to be stored for extrapolation.
	if (config_info -> totally_first_step_bool == 0)
	{
		#pragma omp parallel for
		for (int i = 0; i < NO_OF_VECTORS; ++i)
		{
			interpolation -> pressure_gradient_0_old_m[i] = diagnostics -> pressure_gradient_0_m[i];
			interpolation -> pressure_gradient_1_old[i] = diagnostics -> pressure_gradient_1_dry[i] + diagnostics -> pressure_gradient_1_vapour[i];
		}
	}
	else
	{
		#pragma omp parallel for
		for (int i = 0; i < NO_OF_VECTORS; ++i)
		{
			interpolation -> pressure_gradient_0_old_m[i] = 0;
			interpolation -> pressure_gradient_1_old[i] = 0;
		}
	}
	// The new pressure gradient os only calculated at the first RK step.
	#pragma omp parallel for
	for (int i = 0; i < NO_OF_SCALARS; ++i)
	{
		if (config_info -> tracers_on == 1)
			diagnostics -> c_h_p_field[i] = spec_heat_cap_diagnostics_p(current_state -> density_dry[i], current_state -> tracer_densities[NO_OF_CONDENSATED_TRACERS*NO_OF_SCALARS + i]);
		else
			diagnostics -> c_h_p_field[i] = C_D_P;
	}
	scalar_times_grad(diagnostics -> c_h_p_field, current_state -> temp_gas, diagnostics -> pressure_gradient_0_m, grid);
	scalar_times_grad(diagnostics -> pressure_gradient_1_dry_prefactor, diagnostics -> specific_entropy_dry, diagnostics -> pressure_gradient_1_dry, grid);
	scalar_times_grad(diagnostics -> pressure_gradient_1_vapour_prefactor, diagnostics -> specific_entropy_vapour, diagnostics -> pressure_gradient_1_vapour, grid);
	for (int i = 0; i < NO_OF_VECTORS; ++i)
	{
		forcings -> pressure_gradient_acc[i] = old_hor_grad_weight*(-interpolation -> pressure_gradient_0_old_m[i] + interpolation -> pressure_gradient_1_old[i]) + new_hor_grad_weight*(-diagnostics -> pressure_gradient_0_m[i] + diagnostics -> pressure_gradient_1_dry[i] + diagnostics -> pressure_gradient_1_vapour[i]);
	}
	// The pressure gradient has to get a deceleration factor in presence of condensates.
	double total_density;
	if (config_info -> tracers_on == 1)
	{
		#pragma omp parallel for private(rho_h, total_density)
		for (int i = 0; i < NO_OF_SCALARS; ++i)
		{
			rho_h = current_state -> density_dry[i] + current_state -> tracer_densities[NO_OF_CONDENSATED_TRACERS*NO_OF_SCALARS + i];
		    total_density = current_state -> density_dry[i];
		    for (int k = 0; k < NO_OF_TRACERS; ++k)
		    {
		        total_density += current_state -> tracer_densities[k*NO_OF_SCALARS + i];
	        }
			diffusion_info -> pressure_gradient_decel_factor[i] = rho_h/total_density;
		}
		scalar_times_vector(diffusion_info -> pressure_gradient_decel_factor, forcings -> pressure_gradient_acc, forcings -> pressure_gradient_acc, grid);
	}
	return 0;
}















	
