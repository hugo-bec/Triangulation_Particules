#include <cmath>
#include <cstdio>
#include <chrono>
#include <iostream>
#include <algorithm>
#include "GL/gl3w.h"
#include "imgui_impl_glut.h"

#include "Point.hpp"
#include "define.hpp"
#include "Tetrahedron.hpp"



namespace SIM_PART
{
	void Point::add_tetrahedron( Tetrahedron * t )
	{ _tetras.emplace_back( t->get_id() ); }

	void Point::set_coord( const float px, const float py, const float pz )
	{
		_coord[ 0 ] = px;
		_coord[ 1 ] = py;
		_coord[ 2 ] = pz;
		update_transformation_matrix();
	}

	void Point::update_transformation_matrix() 
	{
		_model._transformation = MAT4F_ID;

		_model._transformation[ 3 ][ 0 ] = 0;
		_model._transformation[ 3 ][ 1 ] = 0;
		_model._transformation[ 3 ][ 2 ] = 0;

		_model._transformation
			= glm::translate( _model._transformation, Vec3f( _coord[ 0 ], _coord[ 1 ], _coord[ 2 ] ) );
		_model._transformation = glm::scale( _model._transformation, _model_scale );
		
	}

	bool Point::is_attract( Point * p, float attract_distance ) const
	{
		const float * p_coord = p->get_coord();
		float		  dx	  = p_coord[ 0 ] - _coord[ 0 ];
		float		  dy	  = p_coord[ 1 ] - _coord[ 1 ];
		float		  dz	  = p_coord[ 2 ] - _coord[ 2 ];
		return ( dx * dx + dy * dy + dz * dz <= attract_distance * attract_distance);
	}

	void Point::compute_coloration( int mode_type )
	{
		int modulo = 6000;
		switch(mode_type)
		{
		case 0:
			if (_attract) set_color( Vec3f(1,0,0) );
			else set_color( Vec3f(0.5) );
			break;
		case 1:
			if ( _fix )
			{
				float lf = _nb_frame_free % modulo / float(modulo);
				float r, g, b;

				float lf_case = lf * 6;
				int	  lf_case_int = int( lf_case );
				float lf_case_reste = lf_case - lf_case_int;

				switch (int(lf_case))
				{
				case 0: r = 1; g = lf_case_reste; b = 0; break;
				case 1: r = 1-lf_case_reste; g = 1; b = 0; break;
				case 2: r = 0; g = 1; b = lf_case_reste; break;
				case 3: r = 0; g = 1-lf_case_reste; b = 1; break;
				case 4: r = lf_case_reste; g = 0; b = 1; break;
				case 5: r = 1; g = 0; b = 1-lf_case_reste; break;
				}

				set_color( Vec3f( r, g, b ) );
				//set_color( Vec3f(0, 1, 0) );
			}
			else set_color( Vec3f(0.5) );
			break;
		}
	}
	
	void Point::tri_voisin()
	{
		sort( _neighbours.begin(), _neighbours.end() );
		auto last = std::unique( _neighbours.begin(), _neighbours.end() );
		_neighbours.erase( last,_neighbours.end() );
	}


	void Point::compute_point_attract_parallelisable_brut( float						   r,
														 const std::vector<Point *> & point_list,
														 int							 refresh_mesh)
	{
		int nb = 0;
		for ( int i = 0; i < point_list.size(); i++ )
		{
			if ( i != _id )
			{
				/* float radius_futur = r + 2 * _speed * refresh_mesh;
				if ( is_attract( point_list[ i ], radius_futur ) )
				{*/
					_possible_futur_attract.emplace_back( point_list[ i ]->_id );

					if ( is_attract( point_list[ i ], r ) )
					{
						//particules_attract.emplace_back( i );
						nb++;
					}
				//}
			}
		}
		std::cout << "nb attract brut " << nb << std::endl;
			
			
	}

	void Point::compute_point_attract_parallelisable_without_double_radius(float							r,
																			const std::vector<Point *> & pointList,
																			std::vector<int> &				traveled_point)
	{
		_points_attract.clear();
		traveled_point[ _id ] = _id;
		for ( int i = 0; i < _neighbours.size(); i++ )
		{
			_points_attract.emplace_back( _neighbours[ i ] );
			traveled_point[ _neighbours[ i ] ] = _id;
		}


		int i = 0;
		while ( i < _points_attract.size()) 
		{
			Point * p = pointList[ _points_attract[ i ] ];
			if ( is_attract( p, r ) )
			{
				i++;
				for ( int j = 0; j < p->get_neighbours()->size(); j++ )
				{
					if ( traveled_point[ ( *p->get_neighbours() )[ j ] ] != _id )
					{
						_points_attract.emplace_back( ( *p->get_neighbours() )[ j ] );
						traveled_point[ ( *p->get_neighbours() )[ j ] ] = _id;
					}
				}
			}
			
			else
				_points_attract.erase( _points_attract.begin() + i );

			
		}
	}

	void Point::compute_point_attract_parallelisable_double_radius( float							r,
															const std::vector<Point *> & pointList,
															std::vector<int> &				traveled_point,
															int								refresh_mesh )
	{
		_points_attract.clear();
		_possible_futur_attract.clear();
		traveled_point[ _id ] = _id;

		for ( int i = 0; i < _neighbours.size(); i++ )
		{
			_possible_futur_attract.emplace_back( _neighbours[ i ] );
			traveled_point[ _neighbours[ i ] ] = _id;
		}
		float radius_futur = r + 2 * _speed * refresh_mesh;
		int i = 0;
		 while ( i < _possible_futur_attract.size() )
		{
			Point * p = pointList[ _possible_futur_attract[ i ] ];
			if ( is_attract( p, radius_futur ) )
			{
				i++;
				const std::vector<int>* p_neighbours = p->get_neighbours();
				for ( int j = 0; j < p_neighbours->size(); j++ )
				{
					if ( traveled_point[ ( *p_neighbours )[ j ] ] != _id )
					{
						_possible_futur_attract.emplace_back( ( *p_neighbours )[ j ] );
						traveled_point[ ( *p_neighbours )[ j ] ] = _id;
					}
				}

				if ( is_attract( p, r ) )
					_points_attract.emplace_back( p->get_id() );
			}

			else
				_possible_futur_attract.erase( _possible_futur_attract.begin() + i );
		}
	}

	float Point::compute_distance( Point * point ) const
	{
		const float* p_coord = point->get_coord();
		float dx = p_coord[ 0 ] - _coord[ 0 ];
		float dy = p_coord[ 1 ] - _coord[ 1 ];
		float dz = p_coord[ 2 ] - _coord[ 2 ];
		return sqrt( dx * dx + dy * dy + dz * dz );
	}

	float Point::compute_distance_squared( Point * point ) const
	{
		const float * p_coord = point->get_coord();
		float dx = p_coord[ 0 ] - _coord[ 0 ];
		float dy = p_coord[ 1 ] - _coord[ 1 ];
		float dz = p_coord[ 2 ] - _coord[ 2 ];
		return  dx * dx + dy * dy + dz * dz;
		
	}

	void Point::compute_attract_by_double_radius( const float					 rayon,
													 const std::vector<Point *> & point_list,
													 std::vector<int> &				 traveled_point,
													 int							 iteration,
													 int							 refresh_frame )
	{
		
		
		for ( int i = 0; i < _points_attract.size(); i++ )
		{
			if ( !is_attract( point_list[ _points_attract[ i ] ], rayon ) )
			{
				_points_attract.erase( _points_attract.begin() + i );
				i--;
			}
		}
	
		Point * p;
		int possible_futur_attract_size = _possible_futur_attract.size();
		for ( int i = 0; i < possible_futur_attract_size; i++ )
		{
				p = point_list[ _possible_futur_attract[ i ] ];
				if ( this->is_attract( p, rayon ) )
				{
					this->_points_attract.push_back( p->_id );
					
				}
		}

		sort( _points_attract.begin(), _points_attract.end() );
		auto last = std::unique( _points_attract.begin(), _points_attract.end() );
		_points_attract.erase( last, _points_attract.end() );

		
	}
	

	void Point::compute_diffusion_limited_aggregation( const float				   rayon, 
														  const std::vector<Point *> & pointList,
														  int					nb_non_fix,
														  int					time_frame)
	{
		if ( ( nb_non_fix <= pointList.size() / 2 && !_fix ) || ( nb_non_fix > pointList.size() / 2 && _fix ) )
		{
			_points_attract.clear();

			Point * p;
			for ( int i = 0; i < _possible_futur_attract.size(); i++ )
			{
				p = pointList[ _possible_futur_attract[ i ] ];
				if ( this->is_attract( p, rayon ) )
				{
					this->_points_attract.push_back( p->_id );
					if ( _fix )
						p->set_fix( time_frame );
					else if ( p->is_fix() )
						set_fix( time_frame );
				}
			}

			sort( _points_attract.begin(), _points_attract.end() );
			auto last = std::unique( _points_attract.begin(), _points_attract.end() );
			_points_attract.erase( last, _points_attract.end() );
		}
		
	}



	void Point::compute_attract_by_flooding( const float					 rayon, 
												const std::vector<Point *> & pointList,
												 std::vector<int>	  traveled_point,
												 int				  iteration,
												 int				  refresh_frame,
												 int				  degre_voisinage)
	{
		for ( int i = 0; i < _points_attract.size(); i++ )
		{
			if ( !is_attract( pointList[ _points_attract[ i ] ], rayon ) )
			{
				_points_attract.erase( _points_attract.begin() + i );
				i--;
			}
		}
		traveled_point[ _id ] = _id;

		std::vector<int> n = _points_attract;
		std::vector<int> n2;
		Point *		 p;
		while (degre_voisinage != 0) 
		{

			for (int i = 0; i < n.size(); i++) 
			{
				p  = pointList[ n[ i ] ];
				if ( this->is_attract( p, rayon ) )
				{
					this->_points_attract.emplace_back( p->get_id() );
				}
				
				const std::vector<int> * tmp = pointList[ n[ i ] ]->get_neighbours();
				for ( int j = 0; j < tmp->size(); j++ ) 
				{
					if ( traveled_point[ (*tmp)[ j ] ] != _id )
					{
						n2.push_back( (*tmp)[ j ] );
						traveled_point[ (*tmp)[ j ] ] = _id;
					}	

				}
			}

			n.clear();
			n = n2;
			n2.clear();
			degre_voisinage--;
		}

		sort( _points_attract.begin(), _points_attract.end() );
		auto last = std::unique( _points_attract.begin(), _points_attract.end() );
		_points_attract.erase( last, _points_attract.end() );
	}

	
	
} // namespace SIM_PART