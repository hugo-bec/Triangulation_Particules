#include "DelaunayStructure.hpp"
#include "utils/random.hpp"
#include "utils/Chrono.hpp"
#include <chrono>

namespace SIM_PART
{
	void DelaunayStructure::init_particules( unsigned int p_nbparticules )
	{
		_nbparticules = p_nbparticules;
		std::cout << "Number of particles: " << _nbparticules << std::endl;

		for ( int i = 0; i < _nbparticules; i++ )
		{
			_positions.push_back(
				Vec3f( getRandomFloat() * _dimCage.x, getRandomFloat() * _dimCage.y, getRandomFloat() * _dimCage.z ) );
			_traveled_point.push_back( -1 );
		}

		_tetgen_mesh.initialize();
		_tetgen_mesh.numberofpoints = _nbparticules;
		_tetgen_mesh.pointlist		 = new REAL[ _tetgen_mesh.numberofpoints * 3 ];

		for ( int i = 0; i < _nbparticules; i++ )
		{
			_tetgen_mesh.pointlist[ 3 * i ]		= _positions[ i ].x;
			_tetgen_mesh.pointlist[ 3 * i + 1 ] = _positions[ i ].y;
			_tetgen_mesh.pointlist[ 3 * i + 2 ] = _positions[ i ].z;

			list_points.push_back( new Point( i, _positions[ i ].x, _positions[ i ].y, _positions[ i ].z ) );

			_colors.push_back(Vec3f( 0 ) );
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

	void DelaunayStructure::tetrahedralize_particules( char * tetgen_parameters )
	{
		_tetgen_mesh.numberofpoints = _nbparticules;
		_tetgen_mesh.pointlist = _tetgen_mesh.pointlist;
		std::cout << "TETGEN: Tetrahedralization..." << std::endl;

		tetrahedralize( tetgen_parameters, &_tetgen_mesh, &_tetgen_mesh );

		std::cout << "number of tetrahedron: " <<  _tetgen_mesh.numberoftetrahedra << std::endl;
		std::cout << "number of points: " << _tetgen_mesh.numberofpoints << std::endl;
	}

	
	void DelaunayStructure::update_tetras()
	{
		list_tetras.clear();
		for ( int j = 0; j < _tetgen_mesh.numberoftetrahedra; j++ )
			list_tetras.push_back( new Tetrahedron( j,
										_tetgen_mesh.tetrahedronlist[ j * 4 ],
										_tetgen_mesh.tetrahedronlist[ j * 4 + 1 ],
										_tetgen_mesh.tetrahedronlist[ j * 4 + 2 ],
										_tetgen_mesh.tetrahedronlist[ j * 4 + 3 ] ) );

		// Computing neighbours for each points
		_chrono.start();
		std::cout << "Adding neighbours from tetrahedrization..." << std::endl;
		for ( int j = 0; j < list_tetras.size(); j++ )
		{
			const std::vector<int>* lp = list_tetras[ j ]->getPoints();
			list_points[ lp->at(0) ]->addTetrahedron( list_tetras[ j ] );
			list_points[ lp->at(1) ]->addTetrahedron( list_tetras[ j ] );
			list_points[ lp->at(2) ]->addTetrahedron( list_tetras[ j ] );
			list_points[ lp->at(3) ]->addTetrahedron( list_tetras[ j ] );

			list_points[ lp->at( 0 ) ]->addNeighbour( lp->at( 1 ) );
			list_points[ lp->at( 0 ) ]->addNeighbour( lp->at( 2 ) );
			list_points[ lp->at( 0 ) ]->addNeighbour( lp->at( 3 ) );

			list_points[ lp->at( 1 ) ]->addNeighbour( lp->at( 0 ) );
			list_points[ lp->at( 1 ) ]->addNeighbour( lp->at( 2 ) );
			list_points[ lp->at( 1 ) ]->addNeighbour( lp->at( 3 ) );

			list_points[ lp->at( 2 ) ]->addNeighbour( lp->at( 0 ) );
			list_points[ lp->at( 2 ) ]->addNeighbour( lp->at( 1 ) );
			list_points[ lp->at( 2 ) ]->addNeighbour( lp->at( 3 ) );

			list_points[ lp->at( 3 ) ]->addNeighbour( lp->at( 0 ) );
			list_points[ lp->at( 3 ) ]->addNeighbour( lp->at( 1 ) );
			list_points[ lp->at( 3 ) ]->addNeighbour( lp->at( 2 ) );
		}

		for ( int k = 0; k < list_points.size(); k++ ) 
			list_points[ k ]->tri_voisin();	
		std::cout << std::endl;

		_chrono.stop_and_print( "Time adding neighbours:" );
	}

	void DelaunayStructure::compute_attract_points()
	{
		std::vector<int> traveled_points( _nbparticules, -1 );
		
		for ( int i = 0; i < (int)list_points.size(); i++ )
		{
			list_points[ i ]->computePointAttractV4( rayon_attract, list_points, traveled_points, refresh_frame );
			if ( i % 1000 == 0 )
				std::cout << "compute attract points: " << i + 1000 << " / " << _nbparticules << "\r";
		}
		std::cout << std::endl;
	}

	void DelaunayStructure::update_structure()
	{
		_chrono.start();
		update_tetras();
		_chrono.stop_and_print( "time reading and interpreting tetgenio: ");

		// Computing attract points for each points
		std::cout << "Computing attract points from tetrahedrization..." << std::endl;
		_chrono.start();
		compute_attract_points();
		_chrono.stop_and_print( "time computing attract points for each points: " );

		// Assign position of the point to OpenGL
		_chrono.start();
		const float * coord;
		for ( int i = 0; i < _nbparticules; i++ ) {
			coord = list_points[ i ]->getCoord();
			this->_positions.push_back( Vec3f( coord[ 0 ], coord[ 1 ], coord[ 2 ] ) );
		}
		_chrono.stop_and_print( "time assigning position: " );
	}


	void DelaunayStructure::update_rendering( bool print_all_edges, int actif_point )
	{
		// edges
		std::vector<int> edges, tmp;
		const std::vector<int> *tp;

		
		if ( !print_all_edges )
		{
			std::vector<int> attract_actif_points = ( *list_points[ actif_point ]->getPointAttract() );
			std::vector<int> tetra_actif_points, list_tetra_tmp;
			for ( int i = 0; i < attract_actif_points.size(); i++ )
			{
				list_tetra_tmp = ( *list_points[ attract_actif_points[ i ] ]->getTetrahedron() );
				tetra_actif_points.insert( tetra_actif_points.end(), list_tetra_tmp.begin(), list_tetra_tmp.end() );
			}
			sort( tetra_actif_points.begin(), tetra_actif_points.end() );
			auto last = std::unique( tetra_actif_points.begin(), tetra_actif_points.end() );
			tetra_actif_points.erase( last, tetra_actif_points.end() );

			for ( int i = 0; i < (int)tetra_actif_points.size(); i++ )
			{
				tp	= list_tetras[ tetra_actif_points[ i ] ]->getPoints();
				tmp = { tp->at( 0 ), tp->at( 1 ), tp->at( 0 ), tp->at( 2 ), tp->at( 0 ), tp->at( 3 ),
						tp->at( 1 ), tp->at( 2 ), tp->at( 1 ), tp->at( 3 ), tp->at( 2 ), tp->at( 3 ) };
				edges.insert( edges.end(), tmp.begin(), tmp.end() );
			}
			this->_indices.clear();
			this->_indices.insert( this->_indices.end(), edges.begin(), edges.end() );
		}
		else 
		{
			for ( int i = 0; i < (int)list_tetras.size(); i++ )
			{
				tp	= list_tetras[ i ]->getPoints();
				tmp = { tp->at( 0 ), tp->at( 1 ), tp->at( 0 ), tp->at( 2 ), tp->at( 0 ), tp->at( 3 ),
						tp->at( 1 ), tp->at( 2 ), tp->at( 1 ), tp->at( 3 ), tp->at( 2 ), tp->at( 3 ) };
				edges.insert( edges.end(), tmp.begin(), tmp.end() );
			}
			this->_indices.clear();
			this->_indices.insert( this->_indices.end(), edges.begin(), edges.end() );
		}
	

		// Change color of attracted point and center point
		if ( list_points[ actif_point ]->getPointAttract()->size() == 0 )
		{
			_indices.push_back( actif_point );
		}
		for ( int i = 0; i < _nbparticules; i++ )
			this->_colors[ i ] = Vec3f( 0 );

		std::vector<int> point_attract = (*list_points[ actif_point ]->getPointAttract());
		for ( int i = 0; i < point_attract.size(); i++ )
			this->_colors[ point_attract[ i ] ] = Vec3f( 1, 0, 0 );
		for (int i = 0; i < 50; i++) 
		{
			this->_colors[ i ] = Vec3f( 0, 1, 0 );
		}
		this->_colors[ actif_point ] = Vec3f( 0, 1, 1 );
		
	}

	void DelaunayStructure::update_position_particules( float speed ) 
	{
		const float * coord;
		for ( int i = 0; i < (int)list_points.size(); i++ )
		{
			list_points[ i ]->apply_brownian_mvt( speed, _dimCage );
			coord = list_points[ i ]->getCoord();

			_positions[ i ].x					= coord[ 0 ];
			_positions[ i ].y					= coord[ 1 ];
			_positions[ i ].z					= coord[ 2 ];

			_tetgen_mesh.pointlist[ i * 3 ]		= coord[ 0 ];
			_tetgen_mesh.pointlist[ i * 3 + 1 ] = coord[ 1 ];
			_tetgen_mesh.pointlist[ i * 3 + 2 ] = coord[ 2 ];

			list_points[ i ]->setCoord( _tetgen_mesh.pointlist[ i * 3 ],
										_tetgen_mesh.pointlist[ i * 3 + 1 ],
										_tetgen_mesh.pointlist[ i * 3 + 2 ] );
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

	void DelaunayStructure::fix_first_points(int nb_points) 
	{ 
		for (int i = 0; i < nb_points; i++) {
			list_points[ i ]->setFix();
		}
	}

	void DelaunayStructure::set_verbose( bool v ) {
		verbose = v;
		_chrono.set_verbose( v );
	}


}