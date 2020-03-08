#include "../../enum_and_typedefs.h"

void recov_hor_par_dual(Dual_vector_field in_field, double out_field[], Grid *grid)
{
    for(int i = 0; i < NUMBER_OF_H_VECTORS; ++i)
    {
        for (int j = 0; j < 11; j++)
            out_field[i] = out_field[i] + grid -> recov_hor_par_dual_weight[11*i + j]*in_field[grid -> recov_hor_par_dual_index[11*i + j]];
    }
}
