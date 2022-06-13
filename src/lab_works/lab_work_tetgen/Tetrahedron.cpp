#include "Tetrahedron.hpp"

namespace tetrasearch
{
	std::vector<int> Tetrahedron::getPoints() { return points; }

	int Tetrahedron::getId() { return id; }

	void Tetrahedron::printPoints( std::vector<Point *> list_points )
	{
		std::vector<float> coord;
		for ( int i = 0; i < (int)points.size(); i++ )
		{
			coord = list_points[ points[ i ] ]->getCoord();
			printf( "id: %i, (x: %1.3f, y: %1.3f, z: %1.3f)\n", 
						list_points[ points[ i ] ]->getId(),
						coord[ 0 ],
						coord[ 1 ],
						coord[ 2 ] );
		}
	}
} // namespace tetrasearch