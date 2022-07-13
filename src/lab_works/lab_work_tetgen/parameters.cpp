#include "parameters.hpp"

namespace SIM_PART
{
	int NB_PARTICULES			= 10000;
	float SPEED_PARTICULES		= 0.1f;
	float ATTRACT_RADIUS		= 0.2f;
	float SIZE_PARTICLE			= 0.1f;
	int	  TETRA_REFRESH_RATE	= 5;

	Vec3f CAGE_DIM				= Vec3f( 10 );
	int	 NB_INIT_FIXED_POINTS	= 1;

	bool DSTRUCTURE_VERBOSE = true;
}