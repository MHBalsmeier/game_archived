/*
This source file is part of the Global Atmospheric Modeling Framework (GAME), which is released under the MIT license.
Github repository: https://github.com/MHBalsmeier/game
*/

#include "../../enum_and_typedefs.h"
#include "../../spatial_operators/spatial_operators.h"

int integrate_temp_gas(State *state_old, State *state_new, Interpolate_info *interpolation, State *state_tendency, Grid *grid, Dualgrid *dualgrid, double delta_t, Scalar_field radiation_tendency, Diagnostics *diagnostics, Forcings *forcings, Diffusion_info *diffusion_info, Config_info *config_info, int no_rk_step)
{   
    scalar_times_vector(state_old -> temp_gas, state_new -> velocity_gas, diagnostics -> temp_gas_flux, grid);
	divv_h(diagnostics -> temp_gas_flux, forcings -> temp_gas_flux_divv_h, grid);
	divv_h(state_new -> velocity_gas, diagnostics -> wind_field_divv_h, grid);
	double total_density, rho_h;
    for (int i = 0; i < NO_OF_SCALARS; ++i)
    {
        if (config_info -> scalar_diffusion_on == 1 && no_rk_step == 2)
        {
            total_density = state_old -> density_dry[i];
            for (int k = 0; k < NO_OF_TRACERS; ++k)
                total_density += state_old -> tracer_densities[k*NO_OF_SCALARS + i];
            rho_h = state_old -> density_dry[i] + state_old -> tracer_densities[NO_OF_CONDENSATED_TRACERS*NO_OF_SCALARS + i];
            // The term with the horizontal wind field divergence is only the explicit component. The implicit component is solved in vertical_slice_solver.
            state_tendency -> temp_gas[i] = -forcings -> temp_gas_flux_divv_h[i] + rho_h/total_density*(diffusion_info -> temp_diffusion_heating[i] + config_info -> momentum_diffusion_on*diffusion_info -> heating_diss[i] + radiation_tendency[i]) + config_info -> tracers_on*config_info -> phase_transitions_on*diffusion_info -> tracer_heat_source_rates[NO_OF_CONDENSATED_TRACERS*NO_OF_SCALARS + i];
        }
        else
        {
            // The term with the horizontal wind field divergence is only the explicit component. The implicit component is solved in vertical_slice_solver.
            state_tendency -> temp_gas[i] = -forcings -> temp_gas_flux_divv_h[i];
        }
    }
	return 0;
}