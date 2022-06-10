#ifndef _TETRA_HPP_
#define _TETRA_HPP_

#include "Point.hpp"
#include <vector>

namespace tetrasearch
{

	class Tetrahedron
	{
	  public:
		Tetrahedron( Point * p1, Point * p2, Point * p3, Point * p4 )
		{
			points.push_back( p1 );
			points.push_back( p2 );
			points.push_back( p3 );
			points.push_back( p4 );
		};
		~Tetrahedron() {};

		std::vector<Point *> getPoints();

	  private:
		std::vector<Point *> points;
	};
} // namespace tetrasearch

#endif