#include "Tetrahedron.hpp"

namespace SIM_PART
{
	const std::vector<int>* Tetrahedron::get_points() { return &points; }

	int Tetrahedron::get_id() { return id; }

	void Tetrahedron::print_points( std::vector<Particle *> list_points )
	{
		const float * coord;
		for ( int i = 0; i < (int)points.size(); i++ )
		{
			coord = list_points[ points[ i ] ]->get_coord();
			printf( "id: %i, (x: %1.3f, y: %1.3f, z: %1.3f)\n", 
						list_points[ points[ i ] ]->get_id(),
						coord[ 0 ],
						coord[ 1 ],
						coord[ 2 ] );
		}
	}
} // namespace tetrasearch