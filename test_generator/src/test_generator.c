/*
This source file is part of the Geophysical Fluids Modeling Framework (GAME), which is released under the MIT license.
Github repository: https://github.com/OpenNWP/GAME
*/

/*
With this program, ideal input states for GAME can be produced.
*/

#include <stdlib.h>
#include "../../src/enum_and_typedefs.h"
#include "../../src/thermodynamics/thermodynamics.h"
#include "../../src/settings.h"
#include "../../src/spatial_operators/spatial_operators.h"
#include "../../src/io/io.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <netcdf.h>
#include "geos95.h"
#include "../../shared/shared.h"
#define NCERR(e) {printf("Error: %s\n", nc_strerror(e)); exit(2);}
#define N_A (6.0221409e23)
#define K_B (1.380649e-23)

// constants needed for the JW test state
const double TROPO_HEIGHT = 12e3;
double T_0 = 288;
double GAMMA = 0.005;
const double G = 9.80616;
const double DELTA_T = 4.8e5;
const double ETA_T = 0.2;
const double U_0 = 35;
const double ETA_0 = 0.252;

int find_pressure_value(double, double, double *);

int main(int argc, char *argv[])
{
	// some thermodynamical quantities
	int TEST_ID;
   	TEST_ID = strtod(argv[1], NULL);
   	int NO_OF_ORO_LAYERS = strtod(argv[2], NULL);
   	const int VERT_GRID_TYPE = strtod(argv[3], NULL);
   	const double TOA = strtof(argv[4], NULL);
   	if (VERT_GRID_TYPE == 1)
   	{
   		NO_OF_ORO_LAYERS = 0;
   	}
   	
   	// determining the orography ID as a function of the test ID
	int ORO_ID;
	if (TEST_ID == 0 || TEST_ID == 3 || TEST_ID == 6)
	{
		ORO_ID = 0;
	}
	if (TEST_ID == 1 || TEST_ID == 4 || TEST_ID == 7)
	{
		ORO_ID = 1;
	}
	if (TEST_ID == 2 || TEST_ID == 5 || TEST_ID == 8)
	{
		ORO_ID = 2;
	}
    int FILE_NAME_LENGTH = 100;
    char *GEO_PROP_FILE_PRE = malloc((FILE_NAME_LENGTH + 1)*sizeof(char));
    sprintf(GEO_PROP_FILE_PRE, "../grid_generator/grids/B%dL%dT%d_O%d_OL%d_SCVT.nc", RES_ID, NO_OF_LAYERS, (int) TOA, ORO_ID, NO_OF_ORO_LAYERS);
    FILE_NAME_LENGTH = strlen(GEO_PROP_FILE_PRE);
    free(GEO_PROP_FILE_PRE);
    char *GEO_PROP_FILE = malloc((FILE_NAME_LENGTH + 1)*sizeof(char));
    sprintf(GEO_PROP_FILE, "../grid_generator/grids/B%dL%dT%d_O%d_OL%d_SCVT.nc", RES_ID, NO_OF_LAYERS, (int) TOA, ORO_ID, NO_OF_ORO_LAYERS);
    Grid *grid = calloc(1, sizeof(Grid));
    Dualgrid *dualgrid = calloc(1, sizeof(Dualgrid));
    set_grid_properties(grid, dualgrid, GEO_PROP_FILE);
    
    int OUTPUT_FILE_LENGTH = 100;
    char *OUTPUT_FILE_PRE = malloc((OUTPUT_FILE_LENGTH + 1)*sizeof(char));
    sprintf(OUTPUT_FILE_PRE, "test_states/test_%d_B%dL%dT%d_O%d_OL%d_SCVT.nc", TEST_ID, RES_ID, NO_OF_LAYERS, (int) TOA, ORO_ID, NO_OF_ORO_LAYERS);
    OUTPUT_FILE_LENGTH = strlen(OUTPUT_FILE_PRE);
    free(OUTPUT_FILE_PRE);
    char *OUTPUT_FILE = malloc((FILE_NAME_LENGTH + 1)*sizeof(char));
    sprintf(OUTPUT_FILE, "test_states/test_%d_B%dL%dT%d_O%d_OL%d_SCVT.nc", TEST_ID, RES_ID, NO_OF_LAYERS, (int) TOA, ORO_ID, NO_OF_ORO_LAYERS);
    double *pressure = malloc(NO_OF_SCALARS*sizeof(double));
    double *temperature = malloc(NO_OF_SCALARS*sizeof(double));
    double *rel_humidity = malloc(NO_OF_SCALARS*sizeof(double));
    double *water_vapour_density = malloc(NO_OF_SCALARS*sizeof(double));
    double *liquid_water_density = malloc(NO_OF_SCALARS*sizeof(double));
    double *solid_water_density = malloc(NO_OF_SCALARS*sizeof(double));
    double *liquid_water_temp = malloc(NO_OF_SCALARS*sizeof(double));
    double *solid_water_temp = malloc(NO_OF_SCALARS*sizeof(double));
    double z_height;
    double lat, lon, u, v, pressure_value, specific_humidity, total_density;
    // dummy arguments
    double dummy_0 = 0.0;
    double dummy_1 = 0.0;
    double dummy_2 = 0.0;
    double dummy_3 = 0.0;
    double dummy_4 = 0.0;
    double dummy_5 = 0.0;
    double dummy_6 = 0.0;
    State *state = calloc(1, sizeof(State));
    int layer_index, h_index;
    int zero = 0;
    int one = 1;
    double one_double = 1;
    // 3D scalar fields determined here, apart from density
    #pragma omp parallel for private(layer_index, h_index, lat, lon, z_height, pressure_value)
    for (int i = 0; i < NO_OF_SCALARS; ++i)
    {
    	layer_index = i/NO_OF_SCALARS_H;
    	h_index = i - layer_index*NO_OF_SCALARS_H;
        lat = grid -> latitude_scalar[h_index];
        lon = grid -> longitude_scalar[h_index];
        z_height = grid -> z_scalar[i];
        rel_humidity[i] = 0;
        // standard atmosphere
        if (TEST_ID == 0 || TEST_ID == 1 || TEST_ID == 2)
        {
            temperature[i] = standard_temp(z_height);
            pressure[i] = standard_pres(z_height);
        }
        // dry Ullrich test
        if (TEST_ID == 3 || TEST_ID == 4 || TEST_ID == 5)
        {
        	baroclinic_wave_test(&one, &zero, &one, &one_double, &lon, &lat, &pressure[i], &z_height, &one, &dummy_0, &dummy_1, &temperature[i], &dummy_2, &dummy_3, &dummy_4, &dummy_5, &dummy_6);
        }
        // moist Ullrich test
        if (TEST_ID == 6 || TEST_ID == 7 || TEST_ID == 8)
        {
        	baroclinic_wave_test(&one, &one, &one, &one_double, &lon, &lat, &pressure[i], &z_height, &one, &dummy_0, &dummy_1, &temperature[i], &dummy_2, &dummy_3, &dummy_4, &total_density, &specific_humidity);
        	water_vapour_density[i] = total_density*specific_humidity;
        }
	    liquid_water_density[i] = 0;
	    solid_water_density[i] = 0;
        liquid_water_temp[i] = temperature[i];
        solid_water_temp[i] = temperature[i];
    }
    Forcings *forcings = calloc(1, sizeof(Forcings));
    
    // reading the grid properties which are not part of the struct grid
    double *latitude_vector = malloc(NO_OF_VECTORS_H*sizeof(double));
    double *longitude_vector = malloc(NO_OF_VECTORS_H*sizeof(double));
    int ncid_grid, retval, latitude_vector_id, longitude_vector_id;
    if ((retval = nc_open(GEO_PROP_FILE, NC_NOWRITE, &ncid_grid)))
        NCERR(retval);
    free(GEO_PROP_FILE);
    if ((retval = nc_inq_varid(ncid_grid, "latitude_vector", &latitude_vector_id)))
        NCERR(retval);
    if ((retval = nc_inq_varid(ncid_grid, "longitude_vector", &longitude_vector_id)))
        NCERR(retval);
    if ((retval = nc_get_var_double(ncid_grid, latitude_vector_id, &latitude_vector[0])))
        NCERR(retval);
    if ((retval = nc_get_var_double(ncid_grid, longitude_vector_id, &longitude_vector[0])))
        NCERR(retval);
    if ((retval = nc_close(ncid_grid)))
        NCERR(retval);

    // horizontal wind fields are determind here
    for (int i = 0; i < NO_OF_LAYERS; ++i)
    {
        for (int j = 0; j < NO_OF_VECTORS_H; ++j)
        {
            lat = latitude_vector[j];
            lon = longitude_vector[j];
            z_height = grid -> z_vector[NO_OF_SCALARS_H + j + i*NO_OF_VECTORS_PER_LAYER];
            // standard atmosphere: no wind
            if (TEST_ID == 0 || TEST_ID == 1 || TEST_ID == 2)
            {
                state -> wind[NO_OF_SCALARS_H + i*NO_OF_VECTORS_PER_LAYER + j] = 0;
            }
            // dry Ullrich test
            if (TEST_ID == 3 || TEST_ID == 4 || TEST_ID == 5)
            {
        		baroclinic_wave_test(&one, &zero, &one, &one_double, &lon, &lat, &dummy_0, &z_height, &one, &u, &v, &dummy_1, &dummy_2, &dummy_3, &dummy_4, &dummy_5, &dummy_6);
                state -> wind[NO_OF_SCALARS_H + i*NO_OF_VECTORS_PER_LAYER + j] = u*cos(grid -> direction[j]) + v*sin(grid -> direction[j]);
            }
            // moist Ullrich test
            if (TEST_ID == 6 || TEST_ID == 7 || TEST_ID == 8)
            {
        		baroclinic_wave_test(&one, &one, &one, &one_double, &lon, &lat, &dummy_0, &z_height, &one, &u, &v, &dummy_1, &dummy_2, &dummy_3, &dummy_4, &dummy_5, &dummy_6);
                state -> wind[NO_OF_SCALARS_H + i*NO_OF_VECTORS_PER_LAYER + j] = u*cos(grid -> direction[j]) + v*sin(grid -> direction[j]);
            }
        }
    }
    free(latitude_vector);
    free(longitude_vector);
    // setting the vertical wind field equal to zero
    for (int i = 0; i < NO_OF_LEVELS; ++i)
    {
    	#pragma omp parallel for private(lat, lon, z_height)
        for (int j = 0; j < NO_OF_SCALARS_H; ++j)
        {
            state -> wind[i*NO_OF_VECTORS_PER_LAYER + j] = 0;
        }
    }    
    
    Diagnostics *diagnostics = calloc(1, sizeof(Diagnostics));
    // this is the density which has not yet been hydrostatically balanced
	for (int i = 0; i < NO_OF_SCALARS; ++i)
	{
		diagnostics -> scalar_field_placeholder[i] = pressure[i]/(specific_gas_constants(0)*temperature[i]);
	}
	scalar_times_vector(diagnostics -> scalar_field_placeholder, state -> wind, diagnostics -> flux_density, grid);
	// Now, the potential vorticity is evaluated.
	calc_pot_vort(state -> wind, diagnostics -> scalar_field_placeholder, diagnostics, grid, dualgrid);
	// Now, the generalized Coriolis term is evaluated.
	vorticity_flux(diagnostics -> flux_density, diagnostics -> pot_vort, forcings -> pot_vort_tend, grid, dualgrid);
	free(dualgrid);
	// Kinetic energy is prepared for the gradient term of the Lamb transformation.
	inner_product(state -> wind, state -> wind, diagnostics -> e_kin, grid);
    // density is determined out of the hydrostatic equation
    int scalar_index;
    double b, c;
    // theta_pert and exner_pert are a misuse of name here, they contain the full values here
	for (int h_index = 0; h_index < NO_OF_SCALARS_H; ++h_index)
	{
		// integrating from bottom to top
		for (int layer_index = NO_OF_LAYERS - 1; layer_index >= 0; --layer_index)
		{
			scalar_index = layer_index*NO_OF_SCALARS_H + h_index;
			// lowest layer
			if (layer_index == NO_OF_LAYERS - 1)
			{
				pressure_value = pressure[scalar_index];
				state -> exner_pert[scalar_index] = pow(pressure_value/P_0, specific_gas_constants(0)/spec_heat_capacities_p_gas(0));
				state -> theta_pert[scalar_index] = temperature[scalar_index]/state -> exner_pert[scalar_index];
			}
			// other layers
			else
			{
				// solving a quadratic equation for the Exner pressure
				b = -0.5*state -> exner_pert[scalar_index + NO_OF_SCALARS_H]/temperature[scalar_index + NO_OF_SCALARS_H]
				*(temperature[scalar_index] - temperature[scalar_index + NO_OF_SCALARS_H]
				+ 2/spec_heat_capacities_p_gas(0)*(grid -> gravity_potential[scalar_index] - grid -> gravity_potential[scalar_index + NO_OF_SCALARS_H]
				+ 0.5*diagnostics -> e_kin[scalar_index] - 0.5*diagnostics -> e_kin[scalar_index + NO_OF_SCALARS_H]
				- (grid -> z_scalar[scalar_index] - grid -> z_scalar[scalar_index + NO_OF_SCALARS_H])*forcings -> pot_vort_tend[h_index + (layer_index + 1)*NO_OF_VECTORS_PER_LAYER]));
				c = pow(state -> exner_pert[scalar_index + NO_OF_SCALARS_H], 2)*temperature[scalar_index]/temperature[scalar_index + NO_OF_SCALARS_H];
				state -> exner_pert[scalar_index] = b + pow((pow(b, 2) + c), 0.5);
			}
			// scalar_field_placeholder is the gas density here
			diagnostics -> scalar_field_placeholder[scalar_index] = P_0*pow(state -> exner_pert[scalar_index],
			spec_heat_capacities_p_gas(0)/specific_gas_constants(0))/(specific_gas_constants(0)*temperature[scalar_index]);
		}
	}
    
    free(forcings);
    int scalar_dimid, vector_dimid, temp_id, density_dry_id, wind_id, density_vapour_id, density_liquid_id, density_solid_id, temperature_liquid_id, temperature_solid_id, ncid, single_double_dimid, stretching_parameter_id;
    if ((retval = nc_create(OUTPUT_FILE, NC_CLOBBER, &ncid)))
        NCERR(retval);
    if ((retval = nc_def_dim(ncid, "scalar_index", NO_OF_SCALARS, &scalar_dimid)))
        NCERR(retval);
    if ((retval = nc_def_dim(ncid, "vector_index", NO_OF_VECTORS, &vector_dimid)))
        NCERR(retval);
    if ((retval = nc_def_dim(ncid, "single_double_dimid_index", 1, &single_double_dimid)))
        NCERR(retval);
    if ((retval = nc_def_var(ncid, "stretching_parameter", NC_DOUBLE, 1, &single_double_dimid, &stretching_parameter_id)))
        NCERR(retval);
    if ((retval = nc_def_var(ncid, "temperature_gas", NC_DOUBLE, 1, &scalar_dimid, &temp_id)))
        NCERR(retval);
    if ((retval = nc_put_att_text(ncid, temp_id, "units", strlen("K"), "K")))
        NCERR(retval);
    if ((retval = nc_def_var(ncid, "density_dry", NC_DOUBLE, 1, &scalar_dimid, &density_dry_id)))
        NCERR(retval);
    if ((retval = nc_put_att_text(ncid, density_dry_id, "units", strlen("kg/m^3"), "kg/m^3")))
        NCERR(retval);
    if ((retval = nc_def_var(ncid, "wind", NC_DOUBLE, 1, &vector_dimid, &wind_id)))
        NCERR(retval);
    if ((retval = nc_put_att_text(ncid, wind_id, "units", strlen("m/s"), "m/s")))
        NCERR(retval);
    if ((retval = nc_def_var(ncid, "density_vapour", NC_DOUBLE, 1, &scalar_dimid, &density_vapour_id)))
        NCERR(retval);
    if ((retval = nc_put_att_text(ncid, density_vapour_id, "units", strlen("kg/m^3"), "kg/m^3")))
        NCERR(retval);
    if ((retval = nc_def_var(ncid, "density_liquid", NC_DOUBLE, 1, &scalar_dimid, &density_liquid_id)))
        NCERR(retval);
    if ((retval = nc_put_att_text(ncid, density_liquid_id, "units", strlen("kg/m^3"), "kg/m^3")))
        NCERR(retval);
    if ((retval = nc_def_var(ncid, "density_solid", NC_DOUBLE, 1, &scalar_dimid, &density_solid_id)))
        NCERR(retval);
    if ((retval = nc_put_att_text(ncid, density_solid_id, "units", strlen("kg/m^3"), "kg/m^3")))
        NCERR(retval);
    if ((retval = nc_def_var(ncid, "temperature_liquid", NC_DOUBLE, 1, &scalar_dimid, &temperature_liquid_id)))
        NCERR(retval);
    if ((retval = nc_put_att_text(ncid, temperature_liquid_id, "units", strlen("T"), "T")))
        NCERR(retval);
    if ((retval = nc_def_var(ncid, "temperature_solid", NC_DOUBLE, 1, &scalar_dimid, &temperature_solid_id)))
        NCERR(retval);
    if ((retval = nc_put_att_text(ncid, temperature_solid_id, "units", strlen("T"), "T")))
        NCERR(retval);
    if ((retval = nc_enddef(ncid)))
        NCERR(retval);
    if ((retval = nc_put_var_double(ncid, stretching_parameter_id, &grid -> stretching_parameter)))
        NCERR(retval);
    if ((retval = nc_put_var_double(ncid, temp_id, &temperature[0])))
        NCERR(retval);
    if ((retval = nc_put_var_double(ncid, density_dry_id, &diagnostics -> scalar_field_placeholder[0])))
        NCERR(retval);
    if ((retval = nc_put_var_double(ncid, wind_id, &state -> wind[0])))
        NCERR(retval);    
    if ((retval = nc_put_var_double(ncid, density_vapour_id, &water_vapour_density[0])))
        NCERR(retval);    
    if ((retval = nc_put_var_double(ncid, density_liquid_id, &liquid_water_density[0])))
        NCERR(retval);
    if ((retval = nc_put_var_double(ncid, density_solid_id, &solid_water_density[0])))
        NCERR(retval);
    if ((retval = nc_put_var_double(ncid, temperature_liquid_id, &liquid_water_temp[0])))
        NCERR(retval);
    if ((retval = nc_put_var_double(ncid, temperature_solid_id, &solid_water_temp[0])))
        NCERR(retval);
    if ((retval = nc_close(ncid)))
    	NCERR(retval);
    free(state);
    free(grid);
    free(pressure);
    free(temperature);
    free(water_vapour_density);
    free(liquid_water_density);
    free(solid_water_density);
    free(liquid_water_temp);
    free(solid_water_temp);
    free(rel_humidity);
    free(OUTPUT_FILE);
    return 0;
}




