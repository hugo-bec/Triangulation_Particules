#include "Tetrahedron.hpp"

namespace SIM_PART
{
	const std::vector<int> & Tetrahedron::get_points() { return points; }

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

	bool Tetrahedron::is_fix( std::vector<Particle *> list_points ) 
	{
		return list_points[ points[ 0 ] ]->is_fix() || list_points[ points[ 1 ] ]->is_fix()
			   || list_points[ points[ 2 ] ]->is_fix() || list_points[ points[ 3 ] ]->is_fix();
	}

} // namespace tetrasearch