/*
This source file is part of the Geophysical Fluids Modeling Framework (GAME), which is released under the MIT license.
Github repository: https://github.com/AUN4GFD/game
*/

/*
Calculates the Held-Suarez radiative forcing.
*/

#include "../enum_and_typedefs.h"
#include "../settings.h"
#include "stdio.h"

double t_eq(double, double);
double k_T(double, double);

int held_suar(double latitude_scalar[], double z_scalar[], double mass_densities[], double temperature_gas[], double radiation_tendency[])
{
	int layer_index, h_index;
	double pressure;
	#pragma omp parallel for private(pressure, layer_index, h_index)
	for (int i = 0; i < NO_OF_SCALARS; ++i)
	{
		layer_index = i/NO_OF_SCALARS_H;
		h_index = i - layer_index*NO_OF_SCALARS_H;
		pressure = mass_densities[NO_OF_CONDENSED_CONSTITUENTS*NO_OF_SCALARS + i]*specific_gas_constants(0)*temperature_gas[i];
		radiation_tendency[i] = -k_T(latitude_scalar[h_index], pressure)*(temperature_gas[i] - t_eq(latitude_scalar[h_index], pressure));
		radiation_tendency[i] = spec_heat_capacities_v_gas(0)*mass_densities[NO_OF_CONDENSED_CONSTITUENTS*NO_OF_SCALARS + i]*radiation_tendency[i];
	}
	return 0;
}

double t_eq(double latitude, double pressure)
{
	double delta_t_y = 60;
	double delta_theta_z = 10;
	double kappa = 2.0/7;
	double result = 315;
	result = result - delta_t_y*pow(sin(latitude), 2);
	result = result - delta_theta_z*log(pressure/P_0)*pow(cos(latitude), 2);
	result = result*pow(pressure/P_0, kappa);
	result = fmax(200, result);
	return result;
}

double k_T(double latitude, double pressure)
{
	double k_a = 1.0/40*1.0/86400;
	double k_s = 1.0/4*1.0/86400;
	double sigma_b = 0.7;
	double sigma = pressure/P_0;
	double result = k_a + (k_s - k_a)*fmax(0, (sigma - sigma_b)/(1 - sigma_b))*pow(cos(latitude), 4);
	return result;
}






