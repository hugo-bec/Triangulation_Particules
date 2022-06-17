#ifndef _TETRA_HPP_
#define _TETRA_HPP_

#include "Point.hpp"
#include <vector>

namespace SIM_PART
{

	class Tetrahedron
	{
	  public:
		Tetrahedron( int _id, int p1, int p2, int p3, int p4 ) : id( _id )
		{
			points.push_back( p1 );
			points.push_back( p2 );
			points.push_back( p3 );
			points.push_back( p4 );
		};
		~Tetrahedron() {};

		std::vector<int> getPoints();
		int				 getId();
		void			 printPoints( std::vector<Point *> list_points );

	  private:
		int				 id;
		std::vector<int> points;
	};
} // namespace tetrasearch

#endif