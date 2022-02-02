/*
This source file is part of the Geophysical Fluids Modeling Framework (GAME), which is released under the MIT license.
Github repository: https://github.com/OpenNWP/GAME
*/

/*
In this file, the gravity potential gets computed.
*/

#include <geos95.h>
#include "../../src/game_types.h"
#include "include.h"

int set_gravity_potential(double z_scalar[], double gravity_potential[], double gravity_mean_sfc_abs, double radius)
{
	/*
	This function computes the gravity potential.
	*/
	
	#pragma omp parallel for
    for (int i = 0; i < NO_OF_SCALARS; ++i)
    {
    	gravity_potential[i] = -gravity_mean_sfc_abs*(radius*radius/(radius + z_scalar[i]) - radius);
    }
	return 0;
}
