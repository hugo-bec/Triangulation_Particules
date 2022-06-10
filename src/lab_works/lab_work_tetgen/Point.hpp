#ifndef _POINT_HPP_
#define _POINT_HPP_

#include <vector>
#include <cstdio>
#include <set>

namespace tetrasearch
{
	class Tetrahedron;

	class Point
	{
	  public:
		Point() {};
		Point( int _id, float _x, float _y, float _z ) : id( _id ), x( _x ), y( _y ), z( _z ) {};

		~Point() {};

		static bool comparatorSet( Point a, Point b ) { return a.samePoints( &b ); }

		std::vector<float>		   getCoord();
		int						   getId();
		std::vector<Tetrahedron *> getTetrahedron();
		std::vector<Point *>	   getPointAttract();
		std::vector<Point *>	   getNeighbours();

		//===========test set================
		// std::set<point, decltype(comparatorSet)> getNeighbours();

		inline void printCoord() { printf( "(x: %lf, y: %lf, z: %lf\n)", x, y, z ); };

		bool isAttract( Point * p, float attract_distance );

		void addPoint( Point * p );

		void searchAdjPoint();

		void computeNeighbours();

		bool samePoints( Point * p );

		void computePointAttract( float r );

		void addTetrahedron( Tetrahedron * t );

	  private:
		int	  id;
		float x;
		float y;
		float z;

		std::vector<Tetrahedron *> tetra;
		std::vector<Point *>	   point_attract;
		std::vector<Point *>	   neighbours;

		//===========test set================
		// std::set<point, decltype(comparatorSet)> neighbours;
	};
} // namespace tetrasearch

#endif