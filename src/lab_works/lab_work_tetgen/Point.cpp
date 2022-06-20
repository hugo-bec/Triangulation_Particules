#include "GL/gl3w.h"
#include "Point.hpp"
#include "Tetrahedron.hpp"
#include <cmath>
#include <cstdio>
#include <iostream>
#include <algorithm>
#include "define.hpp"
#include "imgui_impl_glut.h"
#include <random>

namespace SIM_PART
{

	std::vector<float> Point::getCoord() const
	{
		std::vector<float> coord;
		coord.push_back( this->x );
		coord.push_back( this->y );
		coord.push_back( this->z );

		return coord;
	}

	int	 Point::getId() { return this->id; }
	void Point::addTetrahedron( Tetrahedron * t ) { this->tetra.push_back( t->getId() ); }

	std::vector<int> Point::getTetrahedron() { return this->tetra; }

	std::vector<int> Point::getPointAttract() { return this->point_attract; }

	std::vector<int> Point::getNeighbours() { return this->neighbours; }

	bool Point::isAttract( Point * p, float attract_distance )
	{
		std::vector<float> p_coord = p->getCoord();
		float dx = p_coord[ 0 ] - this->x;
		float dy = p_coord[ 1 ] - this->y;
		float dz = p_coord[ 2 ] - this->z;
		return ( sqrt( dx * dx + dy * dy + dz * dz ) < attract_distance );
	}

	void Point::addPoint( Point * p ) { point_attract.push_back( p->getId() ); }

	bool Point::samePoints( Point * p )
	{
		std::vector<float> pCoords = p->getCoord();

		if ( pCoords[ 0 ] != this->x || pCoords[ 1 ] != this->y || pCoords[ 2 ] != this->z )
			return false;

		else
			return true;
	}

	void Point::computeNeighbours( std::vector<Tetrahedron *> tetraList )
	{
		bool belongs = false;

		Tetrahedron * tetrahedron;

		for ( int i = 0; i < (int)tetra.size(); i++ )
		{
			tetrahedron = findTetra( tetraList, tetra[ i ] );

			for ( int j = 0; j < 4; j++ )
			{
				belongs = false;

				if ( this->id != tetrahedron->getPoints()[ j ] )
				{
					for ( int k = 0; k < (int)neighbours.size(); k++ )
					{
						if ( this->neighbours[ k ] == tetrahedron->getPoints()[ j ] )
						{
							belongs = true;
							break;
						}
					}

					if ( !belongs )
						neighbours.push_back( tetrahedron->getPoints()[ j ] );
				}
			}
		}
	}

	void Point::computeNeighboursV2( std::vector<Tetrahedron *> tetraList )
	{
		Tetrahedron * tetrahedron;

		for ( int i = 0; i < (int)tetra.size(); i++ )
		{
			tetrahedron = findTetra( tetraList, tetra[ i ] );

			for ( int j = 0; j < 4; j++ )
			{
				if ( this->id != tetrahedron->getPoints()[ j ] )
				{
					neighbours.push_back( tetrahedron->getPoints()[ j ] );
				}
			}
		}

		sort( neighbours.begin(), neighbours.end() );
		auto last = std::unique( neighbours.begin(), neighbours.end() );
		neighbours.erase( last, neighbours.end() );
	}


	Point * Point::findPoint( std::vector<Point *> pointList, int id ) { return pointList[ id ]; }

	Tetrahedron * Point::findTetra( std::vector<Tetrahedron *> tetraList, int id ) { return tetraList[ id ]; }

	void Point::computePointAttract( float r, std::vector<Point *> pointList )
	{
		std::vector<int> points			 = this->neighbours;
		std::vector<int> traveled_points = this->neighbours;
		bool			 belongs		 = false;
		Point *			 p;

		while ( points.size() != 0 )
		{
			p = findPoint( pointList, points[ 0 ] );
			if ( this->isAttract( p, r ) )
			{
				this->point_attract.push_back( points[ 0 ] );
				for ( int i = 0; i < (int)p->getNeighbours().size(); i++ )
				{
					belongs = false;

					for ( int j = 0; j < (int)traveled_points.size(); j++ )
					{
						if ( p->getNeighbours()[ i ] == traveled_points[ j ] || this->id == p->getNeighbours()[ i ] )
						{
							belongs = true;
							break;
						}
					}

					if ( !belongs )
					{
						traveled_points.push_back( p->getNeighbours()[ i ] );
						points.push_back( p->getNeighbours()[ i ] );
					}
				}
			}

			points.erase( points.begin() );
		}
	}


	void Point::computePointAttractV4( float				r,
									   std::vector<Point *> pointList,
									   std::vector<int>		traveled_point,
									   int refresh_frame )
	{
		std::vector<int> points = this->neighbours;

		// initialisation du tableau des points parcourus
		for ( int i = 0; i < (int)points.size(); i++ ) {
			traveled_point[ points[ i ] ] = this->id;
		}
		traveled_point[ this->id ] = this->id;
		
		Point * p;
		while ( points.size() != 0 )
		{
			p = pointList[ points[ 0 ] ];
			if ( this->isAttract( p, r + 2 * speed * refresh_frame ) )
			{
				possible_futur_attract.push_back( p->id );
				if ( this->isAttract( p, r ) )
				{
					this->point_attract.push_back( p->id );
					std::vector<int> p_neigbours = p->getNeighbours();
					for ( int i = 0; i < (int)p->getNeighbours().size(); i++ )
					{
						if ( traveled_point[ p_neigbours[ i ] ] != this->id )
						{
							points.push_back( p_neigbours[ i ] );
							traveled_point[ p_neigbours[ i ] ] = this->id;
						}
					}
				}
			}
			points.erase( points.begin() );
		}
	}

    void Point::computePointAttractBrut( float r, std::vector<Point *> pointList )
	{
		int nb = 0;
		possible_futur_attract.clear();
		//std::cout << " Points attract brute :" << std::endl;
		for (int i = 0; i < (int)pointList.size(); i++) 
		{
			if (i!= this->id && this->isAttract(pointList[i], r)) {
				//std::cout << pointList[ i ]->getId() << std::endl;
				possible_futur_attract.push_back( i );
				nb++;
			}
		}
		//std::cout << " Nb Points d'attraction en brute : " <<nb<< std::endl;
	}

	float Point::getDistance(Point* point) 
	{
		std::vector<float> p_coord;
		p_coord = point->getCoord();

		float x = p_coord[ 0 ] - this->x;
		float y = p_coord[ 1 ] - this->y;
		float z = p_coord[ 2 ] - this->z;

		return sqrt( x * x + y * y + z * z );
	}

	void Point::bronien_mvt( float speed, int dimCage ) 
	{ 
		SIM_PART::Vec3f direction;
		direction.x = static_cast<float>( rand() ) / static_cast<float>( RAND_MAX ) * 2 - 1;
		direction.y = static_cast<float>( rand() ) / static_cast<float>( RAND_MAX ) * 2 - 1;
		direction.z = static_cast<float>( rand() ) / static_cast<float>( RAND_MAX ) * 2 - 1;

		if ( ( this->x == 10 && direction.x * speed + this->x > 10 ) || ( this->x == 0 && direction.x * speed + this->x < 0 ) )
			this->x = -direction.x * speed + this->x;
		else if ( direction.x * speed + this->x > 10 )
				this->x = 10;
		else if ( direction.x * speed + this->x < 0 )
				this->x = 0;
		else
			this->x = direction.x * speed + this->x;


		if ( ( this->y == 10 && direction.y * speed + this->y > 10 ) || ( this->y == 0 && direction.y * speed + this->y < 0 ) )
			this->y = -direction.y * speed + this->y;
		else if ( direction.y * speed + this->y > 10 )
			this->y = 10;
		else if ( direction.y * speed + this->y < 0 )
			this->y = 0;
		else
			this->y = direction.y * speed + this->y;


		if ( ( this->z == 10 && direction.z * speed + this->z > 10 ) || ( this->z == 0 && direction.z * speed + this->z < 0 ) )
			this->z = -direction.z * speed + this->z;
		else if ( direction.z * speed + this->z > 10 )
			this->z = 10;
		else if ( direction.z * speed + this->z < 0 )
			this->z = 0;
		else
			this->z = direction.z * speed + this->z;

	}

	//===============test==================


	void Point::computeAttractMethodeDoubleRayon( std::vector<Point *> pointList, std::vector<int>	traveled_point, int iteration, int refresh_frame ) 
	{ 
		std::vector<int> points = this->possible_futur_attract;
		point_attract.clear();

		Point * p;

		while ( points.size() != 0 )
		{
			p = pointList[ points[ 0 ] ];

			if ( this->isAttract( p, rayon ) )
			{
				this->point_attract.push_back( p->id );
			}
			points.erase( points.begin() );
		}
		
	}


	


} // namespace SIM_PART