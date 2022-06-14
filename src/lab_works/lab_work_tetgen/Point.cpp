#include "Point.hpp"
#include "Tetrahedron.hpp"
#include <cmath>
#include <cstdio>
#include <iostream>

namespace tetrasearch
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
		std::vector<float> p_coord;
		p_coord = p->getCoord();

		float x = p_coord[ 0 ] - this->x;
		float y = p_coord[ 1 ] - this->y;
		float z = p_coord[ 2 ] - this->z;

		if ( sqrt( x * x + y * y + z * z ) > attract_distance )
			return false;

		else
			return true;
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

	void Point::computePointAttractV2( float r, std::vector<Point *> pointList )
	{
		std::vector<int> points			 = this->neighbours;
		std::vector<int> traveled_points = this->neighbours;

		int tailleActu;

		int nbPointsAttrac;

		bool	belongs = false;
		Point * p;

		while ( points.size() != 0 )
		{
			nbPointsAttrac = 0;
			tailleActu	   = (int)points.size();
			for ( int i = 0; i < tailleActu; i++ )
			{
				p = findPoint( pointList, points[ i ] );
				if ( this->isAttract( p, r ) )
				{
					this->point_attract.push_back( p->id );
					nbPointsAttrac += 1;
				}
			}
			if ( nbPointsAttrac == tailleActu )
			{
				while ( tailleActu != 0 )
				{
					p = findPoint( pointList, points[ 0 ] );
					for ( int i = 0; i < (int)p->getNeighbours().size(); i++ )
					{
						belongs = false;

						for ( int j = 0; j < (int)traveled_points.size(); j++ )
						{
							if ( p->getNeighbours()[ i ] == traveled_points[ j ]
								 || this->id == p->getNeighbours()[ i ] )
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
					points.erase( points.begin() );
					tailleActu--;
				}

				tailleActu = (int)points.size();
			}

			else
			{
				points.clear();
			}
		}
	}

	void Point::computePointAttractV3( float r, std::vector<Point *> pointList, std::vector<int> traveled_point )
	{
		std::vector<int> points = this->neighbours;

		// initialisation du tableau des points parcourus
		for ( int i = 0; i < (int)points.size(); i++ )
		{
			traveled_point[ points[ i ] ] = this->id;
		}
		traveled_point[ this->id ] = this->id;

		int tailleActu;

		int		nbPointsAttrac;
		Point * p;

		while ( points.size() != 0 )
		{
			nbPointsAttrac = 0;
			tailleActu	   = (int)points.size();

			for ( int i = 0; i < tailleActu; i++ )
			{
				p = findPoint( pointList, points[ i ] );
				if ( this->isAttract( p, r ) )
				{
					this->point_attract.push_back( p->id );
					nbPointsAttrac += 1;
				}
			}

			if ( nbPointsAttrac == tailleActu )
			{
				while ( tailleActu != 0 )
				{
					p = findPoint( pointList, points[ 0 ] );
					for ( int i = 0; i < (int)p->getNeighbours().size(); i++ )
					{
						if ( p->getNeighbours()[ i ] != this->id )
						{
							points.push_back( p->getNeighbours()[ i ] );
						}
					}
					points.erase( points.begin() );
					tailleActu--;
				}

				tailleActu = (int)points.size();
			}

			else
			{
				points.clear();
			}
		}
	}

    

} // namespace tetrasearch