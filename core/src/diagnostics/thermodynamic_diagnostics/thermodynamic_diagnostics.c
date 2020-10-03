/*
This source file is part of the General Geophysical Modeling Framework (GAME), which is released under the MIT license.
Github repository: https://github.com/MHBalsmeier/game
*/
/*
In this source file algebraic conversions and calculations of thermodynamic quantities of a moist atmosphere are collected.
indices as usual:
d:	dry
v:	water vapour
h:	humid
*/

#include "../../enum_and_typedefs.h"
#include "../../spatial_operators/spatial_operators.h"
#include "../diagnostics.h"
#include <stdio.h>
#include <stdlib.h>

int pot_temp_diagnostics(State *state, Scalar_field pot_temp)
{
	/*
	This is only needed for the output.
	*/
	double condensates_density_sum, density_d_micro_value, density_h_micro_value, density_v_micro_value, R_h;
	#pragma omp parallel for private (condensates_density_sum, density_d_micro_value, density_h_micro_value, density_v_micro_value, R_h)
    for (int i = 0; i < NO_OF_SCALARS; ++i)
    {
    	condensates_density_sum = calc_condensates_density_sum(i, state -> tracer_densities);
    	density_d_micro_value = calc_micro_density(state -> density_dry[i], condensates_density_sum);
    	density_v_micro_value = calc_micro_density(state -> tracer_densities[NO_OF_CONDENSATED_TRACERS*NO_OF_SCALARS + i], condensates_density_sum);
		R_h = gas_constant_diagnostics(density_d_micro_value, density_v_micro_value);
    	density_h_micro_value = density_d_micro_value + density_v_micro_value;
    	pot_temp[i] = state -> temperature_gas[i]*pow(P_0/(density_h_micro_value*R_h*state -> temperature_gas[i]), R_D/C_D_P);
	}
    return 0;
}

int temperature_diagnostics(State *state_old, State *state_new)
{
    double nominator, denominator, entropy_density_dry_0, entropy_density_dry_1, density_0, density_1, delta_density, delta_entropy_density, temperature_0, entropy_0, entropy_1;
	#pragma omp parallel for private (nominator, denominator, entropy_density_dry_0, entropy_density_dry_1, density_0, density_1, delta_density, delta_entropy_density, temperature_0, entropy_0, entropy_1)
    for (int i = 0; i < NO_OF_SCALARS; ++i)
    {
    	entropy_density_dry_0 = state_old -> entropy_density_dry[i];
    	entropy_density_dry_1 = state_new -> entropy_density_dry[i];
    	delta_entropy_density = entropy_density_dry_1 - entropy_density_dry_0;
    	density_0 = state_old -> density_dry[i];
    	density_1 = state_new -> density_dry[i];
    	delta_density = density_1 - density_0;
    	temperature_0 = state_old -> temperature_gas[i];
    	entropy_0 = entropy_density_dry_0/density_0;
    	entropy_1 = entropy_density_dry_1/density_1;
    	nominator = C_D_V*density_0*temperature_0 + (R_D*temperature_0 - R_D/C_D_P*entropy_0*temperature_0)*delta_density + R_D/C_D_P*temperature_0*delta_entropy_density;
    	denominator = C_D_V*density_0 + C_D_V/C_D_P*entropy_1*delta_density - C_D_V/C_D_P*delta_entropy_density;
    	state_new -> temperature_gas[i] = nominator/denominator;
    }
    return 0;
}

int temperature_diagnostics_explicit(State *state_old, State *state_tendency, Diagnostics *diagnostics, double delta_t)
{
    double nominator, denominator, entropy_density_dry_0, entropy_density_dry_1, density_0, density_1, delta_density, delta_entropy_density, temperature_0, entropy_0, entropy_1;
	#pragma omp parallel for private (nominator, denominator, entropy_density_dry_0, entropy_density_dry_1, density_0, density_1, delta_density, delta_entropy_density, temperature_0, entropy_0, entropy_1)
    for (int i = 0; i < NO_OF_SCALARS; ++i)
    {
    	entropy_density_dry_0 = state_old -> entropy_density_dry[i];
    	entropy_density_dry_1 = state_old -> entropy_density_dry[i] + delta_t*state_tendency -> entropy_density_dry[i];
    	delta_entropy_density = entropy_density_dry_1 - entropy_density_dry_0;
    	density_0 = state_old -> density_dry[i];
    	density_1 = state_old -> density_dry[i] + delta_t*state_tendency -> density_dry[i];
    	delta_density = density_1 - density_0;
    	temperature_0 = state_old -> temperature_gas[i];
    	entropy_0 = entropy_density_dry_0/density_0;
    	entropy_1 = entropy_density_dry_1/density_1;
    	nominator = C_D_V*density_0*temperature_0 + (R_D*temperature_0 - R_D/C_D_P*entropy_0*temperature_0)*delta_density + R_D/C_D_P*temperature_0*delta_entropy_density;
    	denominator = C_D_V*density_0 + C_D_V/C_D_P*entropy_1*delta_density - C_D_V/C_D_P*delta_entropy_density;
    	diagnostics -> temperature_gas_explicit[i] = nominator/denominator;
    }
    return 0;
}

double spec_heat_cap_diagnostics_p(double density_d_micro_value, double density_v_micro_value)
{
	double result = (density_d_micro_value*C_D_P + density_v_micro_value*C_V_P)/(density_d_micro_value + density_v_micro_value);
	return result;
}

double spec_heat_cap_diagnostics_v(double density_d_micro_value, double density_v_micro_value)
{
	double result = (density_d_micro_value*C_D_V + density_v_micro_value*C_V_V)/(density_d_micro_value + density_v_micro_value);
	return result;
}

double gas_constant_diagnostics(double density_d_micro_value, double density_v_micro_value)
{
	double result = R_D*(1 - density_v_micro_value/(density_d_micro_value + density_v_micro_value) + density_v_micro_value/(density_d_micro_value + density_v_micro_value)*M_D/M_V);
	return result;
}

double entropy_constant_diagnostics(double density_d_micro_value, double density_v_micro_value)
{
	double result = (density_d_micro_value*ENTROPY_CONSTANT_D + density_v_micro_value*ENTROPY_CONSTANT_V)/(density_d_micro_value + density_v_micro_value);
	return result;
}

double calc_micro_density(double density_macro, double condensates_density_sum)
{
	/*
	In a moist atmosphere one needs to distinguish between the densities with respect to the whole volume and the densities with respect to exclusively the gas phase.
	*/
	double result = density_macro/(1 - condensates_density_sum/RHO_WATER);
	if (result < -EPSILON_TRACERS/(1 - condensates_density_sum/RHO_WATER))
	{
		printf("Error: microscopic density negative.\n");
		printf("Aborting.\n");
		exit(0);
	}
	if (isnan(result))
	{
		printf("Error: microscopic density is nan.\n");
		printf("Aborting.\n");
		exit(0);
	}
	return result;
}

double calc_condensates_density_sum(int scalar_gridpoint_index, Tracer_densities tracer_densities)
{
	/*
	This is only needed for calculating the "micro densities".
	*/
	double result = 0;
	for (int i = 0; i < NO_OF_CONDENSATED_TRACERS; ++i)
		result += tracer_densities[i*NO_OF_SCALARS + scalar_gridpoint_index];
	if (result < -NO_OF_CONDENSATED_TRACERS*EPSILON_TRACERS)
	{
		printf("Error: condensates_density_sum negative.\n");
		printf("Aborting.\n");
		exit(0);
	}
	if (result >= RHO_WATER)
	{
		printf("Error: condensates_density_sum >= RHO_WATER.\n");
		printf("Aborting.\n");
		exit(0);
	}
	return result;
}





