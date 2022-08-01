#ifndef _TETRA_HPP_
#define _TETRA_HPP_

#include "Particle.hpp"
#include <vector>

namespace SIM_PART
{
	class Point;

	class Tetrahedron
	{
	  public:
		Tetrahedron( const int _id, const int p1, const int p2, const int p3, const int p4 ) : id( _id )
		{
			points.push_back( p1 );
			points.push_back( p2 );
			points.push_back( p3 );
			points.push_back( p4 );
		}
		~Tetrahedron() {}

		const std::vector<int> & get_points();
		int						get_id();
		void					print_points( std::vector<Point *> list_points );
		bool					is_fix( std::vector<Point *> list_points );

	  private:
		int				 id;
		std::vector<int> points;
	};
} // namespace tetrasearch

#endif