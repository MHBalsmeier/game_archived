#include "../enum_and_typedefs.h"
#include "../r_operators/r_operators.h"
#include "diagnostics.h"
#include <stdio.h>

int pot_temp_diagnostics(Scalar_field density_entropy, Scalar_field density, Add_comp_densities add_comp_densities, Scalar_field pot_temp)
{
	double condensates_density_sum;
	int layer_index, h_index;
    for (int i = 0; i < NUMBER_OF_SCALARS; ++i)
    {
    	layer_index = i/NUMBER_OF_SCALARS_H;
    	h_index = i - layer_index*NUMBER_OF_SCALARS_H;
    	condensates_density_sum = calc_condensates_density_sum(layer_index, h_index, add_comp_densities);
    	pot_temp[i] = pot_temp_diagnostics_single_value(density_entropy[i], density[i], add_comp_densities[NUMBER_OF_COND_ADD_COMPS*NUMBER_OF_SCALARS + i], condensates_density_sum);
	}
    return 0;
}

double pot_temp_diagnostics_single_value(double density_entropy_value, double density_d_value, double density_v_value, double condensates_density_sum)
{
	// for meanings of the numbers f_i, see kompendium
	double density_d_micro_value = calc_micro_density(density_d_value, condensates_density_sum);
	double density_v_micro_value = calc_micro_density(density_v_value, condensates_density_sum);
	double density_h_micro_value = density_d_micro_value + density_v_micro_value;
	double c_h_v = spec_heat_cap_diagnostics_p(density_d_micro_value, density_v_micro_value);
	double R_h = gas_constant_diagnostics(density_d_micro_value, density_v_micro_value);
	double f_1 = density_d_value*C_D_P + density_v_value*C_V_P + density_v_value*M_D/M_V*DELTA_C_V_P*R_D/C_D_V;
	double f_2 = density_d_value*entropy_constant_d + density_v_value*entropy_constant_v;
	double f_3 = density_v_value*M_D/M_V*DELTA_C_V_P*R_D/C_D_V*log(density_h_micro_value*R_h/P_0);
	double result = exp((density_entropy_value - f_2 - f_3)/f_1);
	return result;
}