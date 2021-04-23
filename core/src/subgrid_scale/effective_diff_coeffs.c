/*
This source file is part of the Geophysical Fluids Modeling Framework (GAME), which is released under the MIT license.
Github repository: https://github.com/AUN4GFD/game
*/

/*
In this file, diffusion coefficients, including Eddy viscosities, are computed.
*/

#include "../enum_and_typedefs.h"
#include "../settings.h"
#include "../spatial_operators/spatial_operators.h"
#include "../thermodynamics/thermodynamics.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

int hori_div_viscosity_eff(State *state, Irreversible_quantities *irrev, Grid *grid, Diagnostics *diagnostics, Config_info *config_info, double delta_t)
{
	// these things are hardly ever modified
	double eff_particle_radius = 130e-12;
	double mean_particle_mass = mean_particle_masses_gas(0);
	// the minimum "background" diffusion coefficient
	double min_diff_h_coeff_turb = grid -> mean_area_cell*config_info -> diff_h_smag_fac*config_info -> shear_bg;
	// the maximum diffusion coefficient (stability constraint)
	double max_diff_h_coeff_turb = 0.125*grid -> mean_area_cell/delta_t;
	double molecular_viscosity;
	
	#pragma omp parallel for private(molecular_viscosity)
	for (int i = 0; i < NO_OF_SCALARS; ++i)
	{
		// preliminary result
		irrev -> viscosity_div_eff[i] = 7*grid -> mean_area_cell*config_info -> diff_h_smag_fac
		*fabs(5.0/3*diagnostics -> velocity_gas_divv[i]);
		
		// calculating and adding the molecular viscosity
		calc_diffusion_coeff(state -> temperature_gas[i], mean_particle_mass, state -> mass_densities[NO_OF_CONDENSED_CONSTITUENTS*NO_OF_SCALARS + i],
		eff_particle_radius, &molecular_viscosity);
		irrev -> viscosity_div_eff[i] += molecular_viscosity;
		
		// turbulent minimum
		if (irrev -> viscosity_div_eff[i] < min_diff_h_coeff_turb)
		{
			irrev -> viscosity_div_eff[i] = min_diff_h_coeff_turb;
		}
		
		// maximum (stability constraint)
		if (irrev -> viscosity_div_eff[i] > max_diff_h_coeff_turb)
		{
			irrev -> viscosity_div_eff[i] = max_diff_h_coeff_turb;
		}
		
		// multiplying by the mass density of the gas phase
		irrev -> viscosity_div_eff[i] = density_gas(state, i)*irrev -> viscosity_div_eff[i];
	}
	return 0;
}

int hori_curl_viscosity_eff(State *state, Irreversible_quantities *irrev, Grid *grid, Diagnostics *diagnostics, Config_info *config_info, double delta_t)
{
	// these things are hardly ever modified
	double eff_particle_radius = 130e-12;
	double mean_particle_mass = mean_particle_masses_gas(0);
	// the minimum "background" diffusion coefficient
	double min_diff_h_coeff_turb = grid -> mean_area_cell*config_info -> diff_h_smag_fac*config_info -> shear_bg;
	// the maximum diffusion coefficient (stability constraint)
	double max_diff_h_coeff_turb = 0.125*grid -> mean_area_cell/delta_t;
	double molecular_viscosity;
	
	int layer_index, h_index, scalar_index_from, scalar_index_to;
	#pragma omp parallel for private(molecular_viscosity, layer_index, h_index, scalar_index_from, scalar_index_to)
	for (int i = 0; i < NO_OF_VECTORS; ++i)
	{
		layer_index = i/NO_OF_VECTORS_PER_LAYER;
		h_index = i - layer_index*NO_OF_VECTORS_PER_LAYER;
		if (h_index >= NO_OF_SCALARS_H)
		{
			// preliminary result
			irrev -> viscosity_curl_eff[i] = 0.35*grid -> mean_area_cell*config_info -> diff_h_smag_fac
			*fabs(diagnostics -> rel_vort[NO_OF_VECTORS_H + 2*layer_index*NO_OF_VECTORS_H + h_index - NO_OF_SCALARS_H]);
			
			// calculating and adding the molecular viscosity
			scalar_index_from = layer_index*NO_OF_SCALARS_H + grid -> from_index[h_index - NO_OF_SCALARS_H];
			scalar_index_to = layer_index*NO_OF_SCALARS_H + grid -> to_index[h_index - NO_OF_SCALARS_H];
			calc_diffusion_coeff(
			0.5*(state -> temperature_gas[scalar_index_from] + state -> temperature_gas[scalar_index_to]),
			mean_particle_mass,
			0.5*(state -> mass_densities[NO_OF_CONDENSED_CONSTITUENTS*NO_OF_SCALARS + scalar_index_from] + state -> mass_densities[NO_OF_CONDENSED_CONSTITUENTS*NO_OF_SCALARS + scalar_index_to]),
			eff_particle_radius, &molecular_viscosity);
			irrev -> viscosity_curl_eff[i] += molecular_viscosity;
			
			// turbulent minimum
			if (irrev -> viscosity_curl_eff[i] < min_diff_h_coeff_turb)
			{
				irrev -> viscosity_curl_eff[i] = min_diff_h_coeff_turb;
			}
			
			// maximum (stability constraint)
			if (irrev -> viscosity_curl_eff[i] > max_diff_h_coeff_turb)
			{
				irrev -> viscosity_curl_eff[i] = max_diff_h_coeff_turb;
			}
			
			// multiplying by the mass density of the gas phase
			irrev -> viscosity_curl_eff[i] = 0.5*(density_gas(state, scalar_index_from) + density_gas(state, scalar_index_to))*irrev -> viscosity_curl_eff[i];
		}
	}
	return 0;
}

int vert_hor_mom_viscosity(State *state, Irreversible_quantities *irrev, Diagnostics *diagnostics, Config_info *config_info, Grid *grid, double delta_t)
{
	/*
	This function computes the effective viscosity (Eddy + molecular viscosity) for the vertical diffusion of horizontal velocity.
	This quantity is located at the half level edges.
	To obey the symmetry of the stress tensor, the same coefficient must be used for the horizontal diffusion of vertical velocity.
	*/
	double eff_particle_radius = 130e-12;
	double mean_particle_mass = mean_particle_masses_gas(0);
	double max_diff_v_coeff_turb = 0.125*pow(
	grid -> z_vector[NO_OF_VECTORS - NO_OF_VECTORS_PER_LAYER - NO_OF_SCALARS_H] - grid -> z_vector[NO_OF_VECTORS - NO_OF_SCALARS_H]
	, 2)/delta_t;
	int layer_index, h_index;
	double mom_diff_coeff, molecuar_viscosity, dwdz;
	// loop over horizontal vector points at half levels
	#pragma omp parallel for private(layer_index, h_index, mom_diff_coeff, molecuar_viscosity, dwdz)
	for (int i = 0; i < NO_OF_H_VECTORS - NO_OF_VECTORS_H; ++i)
	{
		layer_index = i/NO_OF_VECTORS_H;
		h_index = i - layer_index*NO_OF_VECTORS_H;
		// calculating the vertical derivative of the vertical velocity
		dwdz = 0.5*(state -> velocity_gas[grid -> from_index[h_index] + layer_index*NO_OF_VECTORS_PER_LAYER] - state -> velocity_gas[grid -> from_index[h_index] + (layer_index + 2)*NO_OF_VECTORS_PER_LAYER])/
		(grid -> z_vector[grid -> from_index[h_index] + layer_index*NO_OF_VECTORS_PER_LAYER] - grid -> z_vector[grid -> from_index[h_index] + (layer_index + 2)*NO_OF_VECTORS_PER_LAYER]);
		dwdz += 0.5*(state -> velocity_gas[grid -> to_index[h_index] + layer_index*NO_OF_VECTORS_PER_LAYER] - state -> velocity_gas[grid -> to_index[h_index] + (layer_index + 2)*NO_OF_VECTORS_PER_LAYER])/
		(grid -> z_vector[grid -> to_index[h_index] + layer_index*NO_OF_VECTORS_PER_LAYER] - grid -> z_vector[grid -> to_index[h_index] + (layer_index + 2)*NO_OF_VECTORS_PER_LAYER]);
		// the turbulent component
		mom_diff_coeff = 1e-3*0.11*pow(
		grid -> z_vector[NO_OF_SCALARS_H + h_index + layer_index*NO_OF_VECTORS_PER_LAYER]
		- grid -> z_vector[NO_OF_SCALARS_H + h_index + (layer_index + 1)*NO_OF_VECTORS_PER_LAYER], 2)
		*sqrt(2*pow(diagnostics -> prep_for_vert_diffusion[i], 2) + pow(dwdz, 2));
		
		// computing and adding the molecular viscosity
		// therefore, the scalar variables need to be averaged to the vector points at half levels
		calc_diffusion_coeff(0.25*(state -> temperature_gas[layer_index*NO_OF_SCALARS_H + grid -> from_index[h_index]]
		+ state -> temperature_gas[layer_index*NO_OF_SCALARS_H + grid -> to_index[h_index]]
		+ state -> temperature_gas[(layer_index + 1)*NO_OF_SCALARS_H + grid -> from_index[h_index]]
		+ state -> temperature_gas[(layer_index + 1)*NO_OF_SCALARS_H + grid -> to_index[h_index]]),
		mean_particle_mass,
		0.25*(state -> mass_densities[NO_OF_CONDENSED_CONSTITUENTS*NO_OF_SCALARS + layer_index*NO_OF_SCALARS_H + grid -> from_index[h_index]]
		+ state -> mass_densities[NO_OF_CONDENSED_CONSTITUENTS*NO_OF_SCALARS + layer_index*NO_OF_SCALARS_H + grid -> to_index[h_index]]
		+ state -> mass_densities[NO_OF_CONDENSED_CONSTITUENTS*NO_OF_SCALARS + (layer_index + 1)*NO_OF_SCALARS_H + grid -> from_index[h_index]]
		+ state -> mass_densities[NO_OF_CONDENSED_CONSTITUENTS*NO_OF_SCALARS + (layer_index + 1)*NO_OF_SCALARS_H + grid -> to_index[h_index]]), eff_particle_radius, &molecuar_viscosity);
		mom_diff_coeff += molecuar_viscosity;
		
		// obeying the stability limit
		if (mom_diff_coeff > max_diff_v_coeff_turb)
		{
			mom_diff_coeff = max_diff_v_coeff_turb;
		}
		
		// multiplying by the density (averaged to the half level edge)
		irrev -> vert_hor_viscosity_eff[i] = 
		0.25*(density_gas(state, layer_index*NO_OF_SCALARS_H + grid -> from_index[h_index])
		+ density_gas(state, layer_index*NO_OF_SCALARS_H + grid -> to_index[h_index])
		+ density_gas(state, (layer_index + 1)*NO_OF_SCALARS_H + grid -> from_index[h_index])
		+ density_gas(state, (layer_index + 1)*NO_OF_SCALARS_H + grid -> to_index[h_index]))
		*mom_diff_coeff;
	}
	return 0;
}

int vert_w_viscosity_eff(State *state, Grid *grid, Diagnostics *diagnostics, double delta_t)
{
	/*
	This function multiplies scalar_field_placeholder (containing dw/dz) by the diffusion coefficient acting on w because of w.
	*/
	double eff_particle_radius = 130e-12;
	double mean_particle_mass = mean_particle_masses_gas(0);
	// the maximum vertical diffusion coefficient
	double max_diff_v_coeff_turb = 0.125*pow(
	grid -> z_vector[NO_OF_VECTORS - NO_OF_VECTORS_PER_LAYER - NO_OF_SCALARS_H] - grid -> z_vector[NO_OF_VECTORS - NO_OF_SCALARS_H]
	, 2)/delta_t;
	int layer_index, h_index;
	double mom_diff_coeff, molecuar_viscosity;
	#pragma omp parallel for private(mom_diff_coeff, molecuar_viscosity, h_index, layer_index)
	for (int i = 0; i < NO_OF_SCALARS; ++i)
	{
		layer_index = i/NO_OF_SCALARS_H;
		h_index = i - layer_index*NO_OF_SCALARS_H;
		// this is the value resulting from turbulence
		mom_diff_coeff = 0.11*pow(
		grid -> z_vector[h_index + layer_index*NO_OF_VECTORS_PER_LAYER] - grid -> z_vector[h_index + (layer_index + 1)*NO_OF_VECTORS_PER_LAYER]
		, 2)
		*fabs(diagnostics -> scalar_field_placeholder[i]);
		// computing and adding the molecular momentum diffusion
		calc_diffusion_coeff(state -> temperature_gas[i], mean_particle_mass,
		state -> mass_densities[NO_OF_CONDENSED_CONSTITUENTS*NO_OF_SCALARS + i], eff_particle_radius, &molecuar_viscosity);
		mom_diff_coeff += molecuar_viscosity;
		// this is the stability criterion
		if (mom_diff_coeff > max_diff_v_coeff_turb)
		{
			mom_diff_coeff = max_diff_v_coeff_turb;
		}
		
		diagnostics -> scalar_field_placeholder[i] = density_gas(state, i)*mom_diff_coeff*diagnostics -> scalar_field_placeholder[i];
	}
	return 0;
}

int calc_temp_diffusion_coeffs(State *state, Config_info *config_info, Irreversible_quantities *irreversible_quantities, Diagnostics *diagnostics, double delta_t, Grid *grid)
{
	/*
	This function computes the viscous temperature diffusion coefficient (including Eddys).
	*/
	// The Eddy viscosity coefficient only has to be calculated if it has not yet been done.
	if (config_info -> momentum_diff_h == 0)
	{
		hori_div_viscosity_eff(state, irreversible_quantities, grid, diagnostics, config_info, delta_t);
		hori_curl_viscosity_eff(state, irreversible_quantities, grid, diagnostics, config_info, delta_t);
	}
	// averaging the curl diffusion coefficient from edges to cells
	edges_to_cells(irreversible_quantities -> viscosity_curl_eff, diagnostics -> scalar_field_placeholder, grid);
	double c_g_v;
	#pragma omp parallel for private (c_g_v)
	for (int i = 0; i < NO_OF_SCALARS; ++i)
	{
		c_g_v = spec_heat_cap_diagnostics_v(state, i, config_info);
		irreversible_quantities -> scalar_diffusion_coeff_numerical_h[i] = c_g_v*(irreversible_quantities -> viscosity_div_eff[i] + diagnostics -> scalar_field_placeholder[i]);
		// vertical Eddy viscosity is about four orders of magnitude smaller
		irreversible_quantities -> scalar_diffusion_coeff_numerical_v[i] = 0.0001*irreversible_quantities -> scalar_diffusion_coeff_numerical_h[i];
	}
	return 0;
}






