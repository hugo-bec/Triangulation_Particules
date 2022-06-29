#include "DelaunayStructure.hpp"
#include <chrono>
#include "glm/gtc/type_ptr.hpp"
#include "parameters.hpp"
#include "utils/random.hpp"
#include "utils/Chrono.hpp"



namespace SIM_PART
{
	/* -----------------------------------------------------------------------
	 * ---------------------- INITIALIZATION FUNCTIONS -----------------------
	 * ----------------------------------------------------------------------- */

	void DelaunayStructure::init_particules( const std::vector<Particle* > & p_particules )
	{
		_nbparticules = p_particules.size();
		_list_points  = p_particules;
		std::cout << "Number of particles: " << _nbparticules << std::endl;

		_tetgen_mesh.initialize();
		_tetgen_mesh.numberofpoints = _nbparticules;
		_tetgen_mesh.pointlist		= new REAL[ _tetgen_mesh.numberofpoints * 3 ];

		for ( int i = 0; i < _nbparticules; i++ )
		{
			const float * coord_p = _list_points[ i ]->get_coord();
			_positions.push_back( Vec3f( coord_p[ 0 ], coord_p[ 1 ], coord_p[ 2 ] ) );
			_traveled_point.push_back( -1 );

			_tetgen_mesh.pointlist[ 3 * i ]		= coord_p[ 0 ];
			_tetgen_mesh.pointlist[ 3 * i + 1 ] = coord_p[ 1 ];
			_tetgen_mesh.pointlist[ 3 * i + 2 ] = coord_p[ 2 ];

			_colors.push_back( Vec3f( 0 ) );
		}
	}

	void DelaunayStructure::init_structure()
	{
		char tetgen_param[ 16 ];
		tetgen_param[ 0 ] = 'Q';
		tetgen_param[ 1 ] = '\0';

		_chrono.start();
		tetrahedralize_particules( tetgen_param );
		_chrono.stop_and_print( "time TETGEN tetrahedralize: " );

		_chrono.start();
		update_structure();
		_chrono.stop_and_print( "time update particles: " );
	}

	void DelaunayStructure::init_buffers() 
	{
		glCreateBuffers( 1, &_vboPoints );
		glCreateBuffers( 1, &_vboColors );
		glCreateBuffers( 1, &_ebo );
		glCreateVertexArrays( 1, &_vao );
	}

	void DelaunayStructure::init_all(const std::vector<Particle*>& p_particules) 
	{
		init_particules( p_particules );

		init_structure();
		update_rendering();

		init_buffers();
		update_buffers();
	}
	

	/* -----------------------------------------------------------------------
	 * -------------------------- UPDATE FUNCTIONS ---------------------------
	 * ----------------------------------------------------------------------- */

	void DelaunayStructure::tetrahedralize_particules( char * tetgen_parameters )
	{
		_tetgen_mesh.numberofpoints = _nbparticules;
		_tetgen_mesh.pointlist		= _tetgen_mesh.pointlist;
		std::cout << "TETGEN: Tetrahedralization..." << std::endl;
		
		tetrahedralize( tetgen_parameters, &_tetgen_mesh, &_tetgen_mesh);

		std::cout << "number of tetrahedron: " << _tetgen_mesh.numberoftetrahedra << std::endl;
		std::cout << "number of points: " << _tetgen_mesh.numberofpoints << std::endl;
	}

	void DelaunayStructure::update_tetras()
	{
		_list_tetras.clear();
		for ( int j = 0; j < _tetgen_mesh.numberoftetrahedra; j++ )
			_list_tetras.push_back( new Tetrahedron( j,
										_tetgen_mesh.tetrahedronlist[ j * 4 ],
										_tetgen_mesh.tetrahedronlist[ j * 4 + 1 ],
										_tetgen_mesh.tetrahedronlist[ j * 4 + 2 ],
										_tetgen_mesh.tetrahedronlist[ j * 4 + 3 ] ) );

		// Computing neighbours for each points
		_chrono.start();
		if ( _verbose ) std::cout << "Adding neighbours from tetrahedrization..." << std::endl;
		for ( int j = 0; j < _list_tetras.size(); j++ )
		{
			const std::vector<int> * plp = _list_tetras[ j ]->get_points();
			const std::vector<int> & lp	 = *plp;
			_list_points[ lp[ 0 ] ]->add_tetrahedron( _list_tetras[ j ] );
			_list_points[ lp[ 1 ] ]->add_tetrahedron( _list_tetras[ j ] );
			_list_points[ lp[ 2 ] ]->add_tetrahedron( _list_tetras[ j ] );
			_list_points[ lp[ 3 ] ]->add_tetrahedron( _list_tetras[ j ] );

			_list_points[ lp[ 0 ] ]->add_neighbour( lp[ 1 ] );
			_list_points[ lp[ 0 ] ]->add_neighbour( lp[ 2 ] );
			_list_points[ lp[ 0 ] ]->add_neighbour( lp[ 3 ] );

			_list_points[ lp[ 1 ] ]->add_neighbour( lp[ 0 ] );
			_list_points[ lp[ 1 ] ]->add_neighbour( lp[ 2 ] );
			_list_points[ lp[ 1 ] ]->add_neighbour( lp[ 3 ] );

			_list_points[ lp[ 2 ] ]->add_neighbour( lp[ 0 ] );
			_list_points[ lp[ 2 ] ]->add_neighbour( lp[ 1 ] );
			_list_points[ lp[ 2 ] ]->add_neighbour( lp[ 3 ] );

			_list_points[ lp[ 3 ] ]->add_neighbour( lp[ 0 ] );
			_list_points[ lp[ 3 ] ]->add_neighbour( lp[ 1 ] );
			_list_points[ lp[ 3 ] ]->add_neighbour( lp[ 2 ] );
		}

		for ( int k = 0; k < _list_points.size(); k++ ) 
			_list_points[ k ]->tri_voisin();	
		std::cout << std::endl;

		_chrono.stop_and_print( "Time adding neighbours:" );
	}

	void DelaunayStructure::update_structure()
	{
		_chrono.start();
		update_tetras();
		_chrono.stop_and_print( "time reading and interpreting tetgenio: ");

		// Computing attract points for each points
		if( _verbose ) std::cout << "Computing attract points from tetrahedrization..." << std::endl;
		_chrono.start();
		compute_attract_points();
		_chrono.stop_and_print( "time computing attract points for each points: " );
	}

	void DelaunayStructure::update_rendering()
	{
		// edges
		//std::cout << "_draw_all_edges: " << _draw_all_edges << ", _active_particle: " << _active_particle << std::endl;
		std::vector<int> edges, tmp;
		const std::vector<int> *tp;

		if ( !_draw_all_edges )
		{
			std::vector<int> attract_actif_points = ( *_list_points[ _active_particle ]->get_point_attract() );
			std::vector<int> tetra_actif_points, list_tetra_tmp;
			std::cout << "nb tetra of point 0: " << _list_points[ 0 ]->get_tetrahedron()->size() << std::endl;

			for ( int i = 0; i < attract_actif_points.size(); i++ )
			{
				list_tetra_tmp = ( *_list_points[ attract_actif_points[ i ] ]->get_tetrahedron() );
				tetra_actif_points.insert( tetra_actif_points.end(), list_tetra_tmp.begin(), list_tetra_tmp.end() );
			}

			sort( tetra_actif_points.begin(), tetra_actif_points.end() );
			auto last = std::unique( tetra_actif_points.begin(), tetra_actif_points.end() );
			tetra_actif_points.erase( last, tetra_actif_points.end() );

			for ( int i = 0; i < (int)tetra_actif_points.size(); i++ )
			{
				tp	= _list_tetras[ tetra_actif_points[ i ] ]->get_points();
				tmp = { tp->at( 0 ), tp->at( 1 ), tp->at( 0 ), tp->at( 2 ), tp->at( 0 ), tp->at( 3 ),
						tp->at( 1 ), tp->at( 2 ), tp->at( 1 ), tp->at( 3 ), tp->at( 2 ), tp->at( 3 ) };
				edges.insert( edges.end(), tmp.begin(), tmp.end() );
			}
			this->_indices.clear();
			this->_indices.insert( this->_indices.end(), edges.begin(), edges.end() );
		}
		else 
		{
			for ( int i = 0; i < (int)_list_tetras.size(); i++ )
			{
				tp	= _list_tetras[ i ]->get_points();
				tmp = { tp->at( 0 ), tp->at( 1 ), tp->at( 0 ), tp->at( 2 ), tp->at( 0 ), tp->at( 3 ),
						tp->at( 1 ), tp->at( 2 ), tp->at( 1 ), tp->at( 3 ), tp->at( 2 ), tp->at( 3 ) };
				edges.insert( edges.end(), tmp.begin(), tmp.end() );
			}
			this->_indices.clear();
			this->_indices.insert( this->_indices.end(), edges.begin(), edges.end() );
		}
		std::cout << "indices size: " << _indices.size() << std::endl;
	
		coloration();
	}

	void DelaunayStructure::update_position_particules() 
	{
		const float * coord;
		for ( int i = 0; i < _list_points.size(); i++ )
		{
			coord = _list_points[ i ]->get_coord();

			_positions[ i ].x					= coord[ 0 ];
			_positions[ i ].y					= coord[ 1 ];
			_positions[ i ].z					= coord[ 2 ];

			_tetgen_mesh.pointlist[ i * 3 ]		= coord[ 0 ];
			_tetgen_mesh.pointlist[ i * 3 + 1 ] = coord[ 1 ];
			_tetgen_mesh.pointlist[ i * 3 + 2 ] = coord[ 2 ];

		}

	}

	void DelaunayStructure::update_buffers()
	{
		// VBO Points
		glNamedBufferData( _vboPoints,
						   _positions.size() * sizeof( Vec3f ),
						   _positions.data(),
						   GL_DYNAMIC_DRAW );	//attention!

		// VBO couleurs
		glNamedBufferData( _vboColors,
						   _colors.size() * sizeof( Vec3f ),
						   _colors.data(),
						   GL_DYNAMIC_DRAW );

		// EBO segments
		glNamedBufferData( _ebo,
						   _indices.size() * sizeof( unsigned int ),
						   _indices.data(),
						   GL_DYNAMIC_DRAW );

		// VAO
		GLuint indexVBO_points = 0;
		GLuint indexVBO_colors = 1;

		// liaison VAO avec VBO Points
		glEnableVertexArrayAttrib( _vao, indexVBO_points );
		glVertexArrayAttribFormat( _vao,
								   indexVBO_points,
								   3 /*car Vec3f*/,
								   GL_FLOAT /*car Vec3f*/,
								   GL_FALSE /*non normalis�*/,
								   0 /*aucune sparation entre les elements*/ );

		glVertexArrayVertexBuffer(
			_vao, indexVBO_points, _vboPoints, 0 /*dbute 0*/, sizeof( Vec3f ) );
		
		glVertexArrayAttribBinding( _vao, 0, indexVBO_points );

		// liaisin VAO avec VBO couleurs
		glEnableVertexArrayAttrib( _vao, indexVBO_colors );
		glVertexArrayAttribFormat( _vao,
								   indexVBO_colors,
								   3 /*car Vec3f*/,
								   GL_FLOAT /*car Vec3f*/,
								   GL_FALSE /*non normalis�*/,
								   0 /*aucune sparation entre les lments*/ );
		glVertexArrayVertexBuffer( _vao, indexVBO_colors, _vboColors, 0, sizeof( Vec3f ) );
		// connexion avec le shader (layout(location = 0))
		glVertexArrayAttribBinding( _vao, 1, indexVBO_colors );


		// liaison VAO avec l'EBO
		glVertexArrayElementBuffer( _vao, _ebo );
	}

	void DelaunayStructure::update_all() 
	{
		if ( _play_mode )
		{
			_chrono.set_verbose( _verbose );
			_chrono.stop_and_print( "time rendering: " );
			if ( _verbose ) 
				std::cout << "------------------------------- " << _iteration << " ---------------------------------" << std::endl;

			_chrono.start();
			update_position_particules();
			_chrono.stop_and_print( "time recomputing brownian movement: " );

			if ( _iteration % _refresh_frame == 0 )
			{
				char tetgen_param[ 16 ];
				tetgen_param[ 0 ] = 'Q';
				tetgen_param[ 1 ] = '\0';

				_chrono.start();
				tetrahedralize_particules( tetgen_param );
				_chrono.stop_and_print( "time TETGEN tetrahedralize: " );

				update_structure();
				
			}

			_chrono.start();

			std::cout << "_mode_type " << _mode_type << std::endl; 
			if ( _mode_type == 0 )
			{
				for ( int j = 0; j < _list_points.size(); j++ ) 
					_list_points[ j ]->compute_attract_by_double_radius( _rayon_attract, _list_points, _traveled_point, _iteration, _refresh_frame );
			}
			else
			{
				for ( int j = 0; j < _list_points.size(); j++ )
					_list_points[ j ]->compute_diffusion_limited_aggregation(
						_rayon_attract, _list_points, _traveled_point, _iteration, _refresh_frame );
			}

			int nb = 0;
			for ( int j = 0; j < _list_points.size(); j++ )
			{
				if ( !_list_points[ j ]->is_fix() )
					nb++;
					
			}
			std::cout << "nb non fix : " << nb << std::endl;
			_chrono.stop_and_print( "time compute attract point with double radius: " );
			_iteration++;
		}

		_chrono.set_verbose( _play_mode && _verbose );
		_chrono.start();
		update_rendering();
		_chrono.stop_and_print( "time coloring points and generating edges: " );

		_chrono.start();
		update_buffers();
		_chrono.stop_and_print( "time updates buffers: " );

		_chrono.start(); // for printing time of the rendering
	}


	/* -----------------------------------------------------------------------
	 * --------------------- COMPUTE & RENDER FUNCTIONS ----------------------
	 * ----------------------------------------------------------------------- */
	
	void DelaunayStructure::compute_attract_points()
	{
		std::vector<int> traveled_points( _nbparticules, -1 );

		for ( int i = 0; i < (int)_list_points.size(); i++ )
		{
			//_list_points[ i ]->compute_point_attract_v4( _rayon_attract, _list_points, traveled_points, _refresh_frame );
			_list_points[ i ]->compute_point_attract_brut( _rayon_attract, _list_points );
			if ( _verbose && i % 1000 == 0 )
				std::cout << "compute attract points: " << i + 1000 << " / " << _nbparticules << "\r";
		}
		std::cout << std::endl;
		const std::vector<int> * ap = _list_points[ 0 ]->get_point_attract();
		std::cout << "ap size: " << ap->size() << std::endl;
	}

	void DelaunayStructure::render( GLuint program, GLuint uModelMatrixLoc ) 
	{
		glBindVertexArray( _vao ); /*bind particules VAO with the program*/
		glProgramUniformMatrix4fv(
			program, uModelMatrixLoc, 1, GL_FALSE, glm::value_ptr( _transformation ) );
		glDrawElements( GL_POINTS, _indices.size(), GL_UNSIGNED_INT, 0 ); /*launching pipeline*/
		glBindVertexArray( 0 );	  /*debind VAO*/

		if ( _edges_mode )	//draw edges
		{
			glBindVertexArray( _vao ); /*bind particules VAO with the program*/
			glProgramUniformMatrix4fv(
				program, uModelMatrixLoc, 1, GL_FALSE, glm::value_ptr( _transformation ) );
			glDrawElements( GL_LINES, _indices.size(), GL_UNSIGNED_INT, 0 ); /*launching pipeline*/
			glBindVertexArray( 0 );  /*debind VAO*/
		}
	}


	void DelaunayStructure::coloration() 
	{
		for (int i = 0; i < _nbparticules; i++) {
			_colors[ i ] = Vec3f( 0 );
		}

		std::vector<int> point_attract;
		switch ( _mode_type )
		{
			case 0: 
				point_attract = ( *_list_points[ _active_particle ]->get_point_attract() );
				for ( int i = 0; i < point_attract.size(); i++ )
					this->_colors[ point_attract[ i ] ] = Vec3f( 1, 0, 0 );
				break;

			case 1: 
				for ( int i = 0; i < _nbparticules; i++ )
				{
					if ( _list_points[ i ]->is_fix() )
						this->_colors[ i ] = Vec3f( 0, 1, 0 );
					else
						this->_colors[ i ] = Vec3f( 0 );
				}
				
				break;
			
			default: break;
		}
		if ( _list_points[ _active_particle ]->get_point_attract()->empty() )
			_indices.push_back( _active_particle );

		this->_colors[ _active_particle ] = Vec3f( 0, 1, 1 );
	}

}