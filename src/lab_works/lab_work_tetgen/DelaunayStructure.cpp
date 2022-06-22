#include "DelaunayStructure.hpp"
#include "utils/random.hpp"
#include "../utils/Utils.hpp"
#include <chrono>

namespace SIM_PART
{
	void DelaunayStructure::init_particules( tetgenio * in )
	{
		for ( int i = 0; i < _nbparticules; i++ )
		{
			this->_positions.push_back(
				Vec3f( getRandomFloat() * _dimCage.x, getRandomFloat() * _dimCage.y, getRandomFloat() * _dimCage.z ) );
			_traveled_point.push_back( -1 );
		}

		in->initialize();
		in->numberofpoints = _nbparticules;
		in->pointlist	   = new REAL[ in->numberofpoints * 3 ];

		for ( int i = 0; i < _nbparticules; i++ )
		{
			in->pointlist[ 3 * i ]	   = this->_positions[ i ].x;
			in->pointlist[ 3 * i + 1 ] = this->_positions[ i ].y;
			in->pointlist[ 3 * i + 2 ] = this->_positions[ i ].z;
			_colors.push_back(Vec3f( 0 ) );
		}

		
	}

	void DelaunayStructure::tetrahedralize_particules( tetgenio * in, tetgenio * out )
	{
		out->initialize();

		out->numberofpoints = _nbparticules;

		out->pointlist = in->pointlist;
		std::cout << "TETGEN: Tetrahedralize..." << std::endl;

		char * param = new char[ 5 ];
		param[ 0 ]	 = 'Q';
		param[ 1 ]	 = '\0';
		tetrahedralize( param, in, out );

		printf( "nombre tetrahedre: %d\n", out->numberoftetrahedra );
		printf( "nombre points: %d\n", out->numberofpoints );
	}

	void DelaunayStructure::_createParticules()
	{
		tetgenio out;

		out.initialize();
		init_particules( &tetgenMesh );
		tetrahedralize_particules( &tetgenMesh, &out );
		update_particules( &out );
	}

	void DelaunayStructure::update_points_tetras( tetgenio * out )
	{
		std::chrono::time_point<std::chrono::system_clock> start_neighbours, stop_neighbours;
		list_points.clear();
		list_tetras.clear();

		for ( int i = 0; i < _nbparticules; i++ )
		{
			list_points.push_back( new Point(
				i, out->pointlist[ i * 3 ], out->pointlist[ i * 3 + 1 ], out->pointlist[ i * 3 + 2 ] ) );
		}

		for ( int j = 0; j < out->numberoftetrahedra; j++ )
		{
			list_tetras.push_back( new Tetrahedron( j,
																 out->tetrahedronlist[ j * 4 ],
																 out->tetrahedronlist[ j * 4 + 1 ],
																 out->tetrahedronlist[ j * 4 + 2 ],
																 out->tetrahedronlist[ j * 4 + 3 ] ) );
		}

		// Computing neighbours for each points
		std::cout << "Computing neighbours from tetrahedrization..." << std::endl;
		start_neighbours = std::chrono::system_clock::now();
		for ( int j = 0; j < list_tetras.size(); j++ )
		{
			std::vector<int> lp = list_tetras[ j ]->getPoints();
			list_points[ lp[ 0 ] ]->addTetrahedron( list_tetras[ j ] );
			list_points[ lp[ 1 ] ]->addTetrahedron( list_tetras[ j ] );
			list_points[ lp[ 2 ] ]->addTetrahedron( list_tetras[ j ] );
			list_points[ lp[ 3 ] ]->addTetrahedron( list_tetras[ j ] );

			list_points[ lp[ 0 ] ]->addNeighbour( lp[ 1 ] );
			list_points[ lp[ 0 ] ]->addNeighbour( lp[ 2 ] );
			list_points[ lp[ 0 ] ]->addNeighbour( lp[ 3 ] );

			list_points[ lp[ 1 ] ]->addNeighbour( lp[ 0 ] );
			list_points[ lp[ 1 ] ]->addNeighbour( lp[ 2 ] );
			list_points[ lp[ 1 ] ]->addNeighbour( lp[ 3 ] );

			list_points[ lp[ 2 ] ]->addNeighbour( lp[ 0 ] );
			list_points[ lp[ 2 ] ]->addNeighbour( lp[ 1 ] );
			list_points[ lp[ 2 ] ]->addNeighbour( lp[ 3 ] );

			list_points[ lp[ 3 ] ]->addNeighbour( lp[ 0 ] );
			list_points[ lp[ 3 ] ]->addNeighbour( lp[ 1 ] );
			list_points[ lp[ 3 ] ]->addNeighbour( lp[ 2 ] );
		}


		for ( int k = 0; k < list_points.size(); k++ ) 
		{
			list_points[ k ]->tri_voisin();	
		}

		std::cout << std::endl;
		stop_neighbours = std::chrono::system_clock::now();
		std::chrono::duration<double> time_neighbours = stop_neighbours - start_neighbours;
		std::cout << "Time computing neighbours: \t\t" << time_neighbours.count() << "s" << std::endl;
	}

	void DelaunayStructure::compute_neighbours()
	{
		for ( int i = 0; i < (int)list_points.size(); i++ )
		{
			list_points[ i ]->computeNeighboursV2( list_tetras );
			std::cout << "compute neighbours: " << i+1 << " / " << _nbparticules << "\r";
		}
	}

	void DelaunayStructure::compute_attract_points()
	{
		std::vector<int> traveled_points( _nbparticules, -1 );

		for ( int i = 0; i < (int)list_points.size(); i++ )
		{
			list_points[ i ]->computePointAttractV4( rayon_attract, list_points, traveled_points, refresh_frame );
			std::cout << "compute attract points: " << i+1 << " / " << _nbparticules << "\r";
		}
	}

	void DelaunayStructure::update_particules( tetgenio * out )
	{
		std::chrono::time_point<std::chrono::system_clock> start_tetra, stop_tetra, start_neighbours, stop_neighbours, start_attract,
			stop_attract;

		start_tetra = std::chrono::system_clock::now();
		update_points_tetras( out );
		stop_tetra = std::chrono::system_clock::now();
		Utils::print_time( "time reading and interpreting tetgenio: ", start_tetra, stop_tetra );

		//compute_neighbours();

		

		// Computing attract points for each points
		std::cout << "Computing attract points from tetrahedrization..." << std::endl;
		start_attract = std::chrono::system_clock::now();

		compute_attract_points();

		std::cout << std::endl;
		stop_attract = std::chrono::system_clock::now();

		// Assign position of the point for OpenGL
		std::vector<float> coord;
		for ( int i = 0; i < _nbparticules; i++ ) {
			coord = list_points[ i ]->getCoord();
			this->_positions.push_back( Vec3f( coord[ 0 ], coord[ 1 ], coord[ 2 ] ) );
		}

		// Computing and printing nb particles and times
		std::chrono::duration<double> time_attract	  = stop_attract - start_attract;
		std::cout << "Number of particles: " << _nbparticules << std::endl;
		std::cout << "Time computing attracted points: \t" << time_attract.count() << "s" << std::endl;
	}


	void DelaunayStructure::_colorPoint( bool print_all_edges, int actif_point )
	{
		// edges
		std::vector<int> edges, tmp, tp;
		if ( !print_all_edges )
		{
			std::vector<int> attract_actif_points = (*list_points[ actif_point ]->getPointAttract());
			std::vector<int> tetra_actif_points, list_tetra_tmp;
			for ( int i = 0; i < attract_actif_points.size(); i++ )
			{
				list_tetra_tmp = (*list_points[ attract_actif_points[ i ] ]->getTetrahedron());
				tetra_actif_points.insert( tetra_actif_points.end(), list_tetra_tmp.begin(), list_tetra_tmp.end() );
			}
			sort( tetra_actif_points.begin(), tetra_actif_points.end() );
			auto last = std::unique( tetra_actif_points.begin(), tetra_actif_points.end() );
			tetra_actif_points.erase( last, tetra_actif_points.end() );

			for ( int i = 0; i < (int)tetra_actif_points.size(); i++ )
			{
				tp	= list_tetras[ tetra_actif_points[ i ] ]->getPoints();
				tmp = { tp[ 0 ], tp[ 1 ], tp[ 0 ], tp[ 2 ], tp[ 0 ], tp[ 3 ],
						tp[ 1 ], tp[ 2 ], tp[ 1 ], tp[ 3 ], tp[ 2 ], tp[ 3 ] };
				edges.insert( edges.end(), tmp.begin(), tmp.end() );
			}
		}
		else
		{
			for ( int i = 0; i < (int)list_tetras.size(); i++ )
			{
				tp	= list_tetras[ i ]->getPoints();
				tmp = { tp[ 0 ], tp[ 1 ], tp[ 0 ], tp[ 2 ], tp[ 0 ], tp[ 3 ],
						tp[ 1 ], tp[ 2 ], tp[ 1 ], tp[ 3 ], tp[ 2 ], tp[ 3 ] };
				edges.insert( edges.end(), tmp.begin(), tmp.end() );
			}
		}
		this->_indices.clear();
		this->_indices.insert( this->_indices.end(), edges.begin(), edges.end() );

		// Change color of attracted point and center point
		

		for ( int i = 0; i < _nbparticules; i++ )
			this->_colors[ i ] = Vec3f( 0 );

		std::vector<int> point_attract = (*list_points[ actif_point ]->getPointAttract());
		for ( int i = 0; i < point_attract.size(); i++ )
			this->_colors[ point_attract[ i ] ] = Vec3f( 1, 0, 0 );

		this->_colors[ actif_point ] = Vec3f( 0, 1, 1 );
	}

	void DelaunayStructure::_initBuffersParticules()
	{
		// VBO Points
		glCreateBuffers( 1, &_vboPoints );
		glNamedBufferData( _vboPoints,
						   _positions.size() * sizeof( Vec3f ),
						   _positions.data(),
						   GL_DYNAMIC_DRAW );	//attention!

		// VBO couleurs
		glCreateBuffers( 1, &_vboColors );
		glNamedBufferData( _vboColors,
						   _colors.size() * sizeof( Vec3f ),
						   _colors.data(),
						   GL_DYNAMIC_DRAW );

		// EBO segments
		//for ( int i = 0; i < ( *part )._positions.size(); i++ ) ( *part )._indices.push_back( i );
		glCreateBuffers( 1, &_ebo );
		glNamedBufferData( _ebo,
						   _indices.size() * sizeof( unsigned int ),
						   _indices.data(),
						   GL_DYNAMIC_DRAW );

		// VAO
		GLuint indexVBO_points = 0;
		GLuint indexVBO_colors = 1;
		glCreateVertexArrays( 1, &_vao );

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


}