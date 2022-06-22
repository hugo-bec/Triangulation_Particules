#ifndef _POINT_HPP_
#define _POINT_HPP_

#include <vector>
#include <cstdio>
#include <set>

namespace SIM_PART
{
	class Tetrahedron;

	// bool comparatorSet(point a, point b){ return a.samePoints(b); }

	class Point
	{
		struct compare
		{
			bool operator()( const Point * pa, const Point * pb ) const
			{
				std::vector<float> a = pa->getCoord();
				std::vector<float> b = pb->getCoord();
				if ( a[ 0 ] != b[ 0 ] || a[ 1 ] != b[ 1 ] || a[ 2 ] != b[ 2 ] )
					return true;
				else
					return false;
				/*return  a[0] != b[0] ? a[0] < b[0] :
							(a[1] != b[1] ? a[1] < b[1] :
								(a[2] != b[2] ? a[2] < b[2] : true ) ); */
			}
		};

	  public:
		Point() {};
		/*Point( float _x, float _y, float _z, std::vector<tetrahedron> _tetra )
		: x( _x ), y( _y ), z( _z ), tetra( _tetra ) {};*/

		Point( int _id, float _x, float _y, float _z ) : id( _id ), x( _x ), y( _y ), z( _z ) {};

		~Point() {};

		// static bool comparatorSet(Point a, Point b){ return a.samePoints(&b); }

		std::vector<float> getCoord() const;
		int				   getId();
		std::vector<int>   getTetrahedron();
		std::vector<int>   getPointAttract();
		std::vector<int>   getNeighbours();
		void			   addNeighbour( int i ) { neighbours.push_back( i ); }
		void			   addPossibleAttract( int i ) { possible_futur_attract.push_back( i ); }
		void			   addAttract( int i ) { point_attract.push_back( i ); }
		void			   tri_voisin();
		
		inline void printCoord() { printf( "(x: %lf, y: %lf, z: %lf\n)", x, y, z ); };

		bool isAttract( Point * p, float attract_distance );

		void addPoint( Point * p );

		//void searchAdjPoint();

		void computeNeighbours( std::vector<Tetrahedron *> tetraList );

		void computeNeighboursV2( std::vector<Tetrahedron *> tetraList );

		bool samePoints( Point * p );

		void computePointAttract( float r, std::vector<Point *> PointList );

		void addTetrahedron( Tetrahedron * t );

		Point * findPoint( std::vector<Point *> PointList, int id );

		Tetrahedron * findTetra( std::vector<Tetrahedron *> PointList, int id );

		

		void computePointAttractV4( float				 r,
									const std::vector<Point *> &PointList,
									std::vector<int>	 &traveled_point,
									int					 refresh_frame );

		//=============Point Attract Version brute============

		void computePointAttractBrut( float r, std::vector<Point *> PointList );

		//=============Trouver point attract sans refaire les tétrahèdres============
		
		void computeAttractMethodeDoubleRayon( const float					rayon, 
											   const std::vector<Point *> &pointList,
											   std::vector<int>		&traveled_point,
											   int					iteration,
											   int					refresh_frame );


		void computeAttractMethodeInondation(	const float				   rayon, 
												const std::vector<Point *> &pointList,
												std::vector<int>	  &traveled_point,
												int				  iteration,
												int				  refresh_frame, 
												int degre_voisinage );
		
		float getDistance( Point * point );

		float getDistance2( Point * point );

		void bronien_mvt( float speed, int dimCage );

		

	  private:
		int	  id;
		float x;
		float y;
		float z;
		float speed = 0.01f;
		//float rayon = 2.f;

		std::vector<int> tetra;
		std::vector<int> point_attract;
		std::vector<int> neighbours;
		std::vector<int> possible_futur_attract;
		int				 taille_attract;

		//===========test set================
		// std::set<point, decltype(comparatorSet)> neighbours;
	};
} // namespace tetrasearch

#endif