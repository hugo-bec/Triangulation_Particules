#include "DelaunayStructure.hpp"
#include <chrono>
#include <omp.h>
#include "glm/gtc/type_ptr.hpp"
#include "parameters.hpp"
#include "utils/random.hpp"
#include "utils/Chrono.hpp"
#include <assimp/postprocess.h>

namespace SIM_PART
{
	/* -----------------------------------------------------------------------
	 * ---------------------- INITIALIZATION FUNCTIONS -----------------------
	 * ----------------------------------------------------------------------- */

	void DelaunayStructure::init_particules( const std::vector<Particle* > & p_particules, int p_refresh_rate )
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
		_refresh_frame = p_refresh_rate;
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
		_chrono.stop_and_print( "time update structure: " );
	}

	void DelaunayStructure::init_mesh()
	{
		const FilePath p_filePath = "data/model/icosphere2.obj";
		Assimp::Importer importer;

		// Importer options
		// Cf. http://assimp.sourceforge.net/lib_html/postprocess_8h.html.
		const unsigned int flags = aiProcessPreset_TargetRealtime_Fast | aiProcess_FlipUVs;
		const aiScene * const scene = importer.ReadFile( p_filePath, flags );
		if ( scene == nullptr )
			throw std::runtime_error( "Fail to load file \" " + p_filePath.str() + "\": " + importer.GetErrorString() );

		_model = scene->mMeshes[ 0 ];

		_mesh_vertices.resize( _model->mNumVertices );
		for ( unsigned int v = 0; v < _model->mNumVertices; ++v )
		{
			Vertex & vertex = _mesh_vertices[ v ];
			// Position.
			vertex._position.x = _model->mVertices[ v ].x;
			vertex._position.y = _model->mVertices[ v ].y;
			vertex._position.z = _model->mVertices[ v ].z;
			// Color.
			vertex._color = Vec3f(0);
			// Normal.
			vertex._normal.x = _model->mNormals[ v ].x;
			vertex._normal.y = _model->mNormals[ v ].y;
			vertex._normal.z = _model->mNormals[ v ].z;
		}

		_mesh_indices.resize( _model->mNumFaces * 3 ); // Triangulated.
		for ( unsigned int f = 0; f < _model->mNumFaces; ++f )
		{
			const aiFace &	   face = _model->mFaces[ f ];
			const unsigned int f3	= f * 3;
			_mesh_indices[ f3 ]		= face.mIndices[ 0 ];
			_mesh_indices[ f3 + 1 ] = face.mIndices[ 1 ];
			_mesh_indices[ f3 + 2 ] = face.mIndices[ 2 ];
		}
		
	}

	void DelaunayStructure::init_buffers() 
	{
		/* ------------------
		 * --- PARTICULES ---
		 * ------------------ */

		glCreateBuffers( 1, &_vboPositions );
		glCreateBuffers( 1, &_vboColors );
		glCreateBuffers( 1, &_eboParticules );
		glCreateVertexArrays( 1, &_vaoParticules );

		// VAO
		GLuint indexVBO_positions = 0;
		GLuint indexVBO_colors = 1;

		// liaison VAO avec VBO Points
		glEnableVertexArrayAttrib( _vaoParticules, indexVBO_positions );
		glVertexArrayAttribFormat( _vaoParticules,
								   indexVBO_positions,
								   3 /*car Vec3f*/,
								   GL_FLOAT /*car Vec3f*/,
								   GL_FALSE /*non normalis�*/,
								   0 /*aucune sparation entre les elements*/ );

		glVertexArrayVertexBuffer( _vaoParticules, indexVBO_positions, _vboPositions, 0 /*dbute 0*/, sizeof( Vec3f ) );
		glVertexArrayAttribBinding( _vaoParticules, indexVBO_positions, indexVBO_positions );

		// liaisin VAO avec VBO couleurs
		glEnableVertexArrayAttrib( _vaoParticules, indexVBO_colors );
		glVertexArrayAttribFormat( _vaoParticules,
								   indexVBO_colors,
								   3 /*car Vec3f*/,
								   GL_FLOAT /*car Vec3f*/,
								   GL_FALSE /*non normalis�*/,
								   0 /*aucune sparation entre les lments*/ );
		glVertexArrayVertexBuffer( _vaoParticules, indexVBO_colors, _vboColors, 0, sizeof( Vec3f ) );
		// connexion avec le shader (layout(location = 0))
		glVertexArrayAttribBinding( _vaoParticules, indexVBO_colors, indexVBO_colors );

		// liaison VAO avec l'EBO
		glVertexArrayElementBuffer( _vaoParticules, _eboParticules );

	}

	void DelaunayStructure::init_all( GLuint program, const std::vector<Particle *> & p_particules, int p_refresh_rate ) 
	{
		init_particules( p_particules, p_refresh_rate );
		init_mesh();

		init_structure();
		update_rendering();

		init_buffers();
		update_buffers();

		_uModelMatrixLoc = glGetUniformLocation( program, "uModelMatrix" );
	}
	

	/* -----------------------------------------------------------------------
	 * -------------------------- UPDATE FUNCTIONS ---------------------------
	 * ----------------------------------------------------------------------- */

	void DelaunayStructure::tetrahedralize_particules( char * tetgen_parameters )
	{
		_tetgen_mesh.numberofpoints = _nbparticules;
		_tetgen_mesh.pointlist		= _tetgen_mesh.pointlist;
		std::cout << "TETGEN: Tetrahedralization..." << std::endl;
		
		tetrahedralize( tetgen_parameters, &_tetgen_mesh, &_tetgen_mesh );

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
		for (int i = 0; i < _list_points.size(); i++) {
			_list_points[ i ]->clear_tetrahedron();
		}
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
			if ( _mode_type == 0 )
				_filtered_points = ( *_list_points[ _active_particle ]->get_point_attract() );
			else if ( _mode_type == 1 )
				for ( int i = 0; i < _nbparticules; i++ )
					if ( _list_points[ i ]->is_fix() )
						_filtered_points.emplace_back( i );

			std::vector<int> tetra_filtered_points, list_tetra_tmp;
			// std::cout << "nb tetra of point 0: " << _list_points[ 0 ]->get_tetrahedron()->size() << std::endl;

			for ( int i = 0; i < _filtered_points.size(); i++ )
			{
				list_tetra_tmp = ( *_list_points[ _filtered_points[ i ] ]->get_tetrahedron() );
				tetra_filtered_points.insert(
					tetra_filtered_points.end(), list_tetra_tmp.begin(), list_tetra_tmp.end() );
			}

			sort( tetra_filtered_points.begin(), tetra_filtered_points.end() );
			auto last = std::unique( tetra_filtered_points.begin(), tetra_filtered_points.end() );
			tetra_filtered_points.erase( last, tetra_filtered_points.end() );

			for ( int i = 0; i < (int)tetra_filtered_points.size(); i++ )
			{
				tp = _list_tetras[ tetra_filtered_points[ i ] ]->get_points();
				tmp.clear();
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
		//std::cout << "indices size: " << _indices.size() << std::endl;
	
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
		/* PARTICULES */
		glNamedBufferData( _vboPositions,
						   _positions.size() * sizeof( Vec3f ),
						   _positions.data(),
						   GL_DYNAMIC_DRAW );	//attention!

		glNamedBufferData( _vboColors,
						   _colors.size() * sizeof( Vec3f ),
						   _colors.data(),
						   GL_DYNAMIC_DRAW );

		glNamedBufferData( _eboParticules,
						   _indices.size() * sizeof( unsigned int ),
						   _indices.data(),
						   GL_DYNAMIC_DRAW );

		for ( int i = 0; i < _colors.size(); i++ )
			_list_points[ i ]->set_color( _colors[ i ] );
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
				_degre_voisinage = 2;
				
			}

			_chrono.start();

			//std::cout << "_mode_type " << _mode_type << std::endl; 
			if ( _mode_type == 0 )
			{
				#pragma omp parallel
				{
					std::vector<int> private_traveled_points( _nbparticules, -1 );
					#pragma omp for
					for ( int j = 0; j < _list_points.size(); j++ )
						//_list_points[ j ]->compute_attract_by_double_radius(_rayon_attract, _list_points, _traveled_point, _iteration, _refresh_frame ); 
						_list_points[ j ]->compute_attract_by_flooding(_rayon_attract, _list_points, private_traveled_points, _iteration, _refresh_frame, _degre_voisinage );
					
				}
				_chrono.stop_and_print( "time compute attract point with double radius parallelisable: " );

				std::cout << "nb point attract double rayon " << _list_points[ 0 ]->get_point_attract()->size()
						  << std::endl;
				
				_list_points[ 0 ]->compute_point_attract_parallelisable_brut(
					_rayon_attract, _list_points, TETRA_REFRESH_RATE );

				if ( _iteration%2==0)
					_degre_voisinage++;

				
				int p = 0;
				const std::vector<int>* voisin;
				_chrono.start();
				for ( int j = 0; j < _list_points.size(); j++ )
				{
					voisin = _list_points[ j ]->get_neighbours();
				}
				_chrono.stop_and_print( "parcours points : " );
			}
			else
			{
				if ( nb_non_fix != 0 )
				{
					for ( int j = 0; j < _list_points.size(); j++ )
						_list_points[ j ]->compute_diffusion_limited_aggregation(
							_rayon_attract, _list_points, _traveled_point, _iteration, _refresh_frame, nb_non_fix );

					nb_non_fix = 0;
					for ( int j = 0; j < _list_points.size(); j++ )
					{
						if ( !_list_points[ j ]->is_fix() )
						{
							nb_non_fix++;
						}
					}
					HANDLE hConsole = GetStdHandle( STD_OUTPUT_HANDLE );
					SetConsoleTextAttribute( hConsole, 2 );
					std::cout <<"Unfixed particle number : " << nb_non_fix << std::endl;
					SetConsoleTextAttribute( hConsole, 7 );
				}
			}

			//_chrono.stop_and_print( "time compute attract point with double radius: " );
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

		#pragma omp parallel
		{
			std::vector<int> traveled_points( _nbparticules, -1 );
			int list_points_size = _list_points.size();
			#pragma omp for
			for ( int i = 0; i < list_points_size; i++ )
			{
				//_list_points[ i ]->compute_point_attract_parallelisable( _rayon_attract, _list_points, TETRA_REFRESH_RATE /*, traveled_points */ );
				//_list_points[ i ]->compute_point_attract_parallelisable_without_double_radius(_rayon_attract, _list_points, traveled_points );
				_list_points[ i ]->compute_point_attract_parallelisable_double_radius(_rayon_attract, _list_points, traveled_points, TETRA_REFRESH_RATE );
				//if ( _verbose && i % 1000 == 0 )
					//std::cout << "compute attract points: " << i + 1000 << " / " << _nbparticules << "\r";
			}
		}
		std::cout << "nb point attract double rayon " << _list_points[ 0 ]->get_point_attract()->size() << std::endl;
		_list_points[ 0 ]->compute_point_attract_parallelisable_brut( _rayon_attract, _list_points, TETRA_REFRESH_RATE );
		
		std::cout << std::endl;
		
	}

	void DelaunayStructure::render( GLuint program ) 
	{
		if ( _edges_mode )	//draw edges
		{
			glBindVertexArray( _vaoParticules ); /*bind particules VAO with the program*/
			glProgramUniformMatrix4fv(
				program, _uModelMatrixLoc, 1, GL_FALSE, glm::value_ptr( _transformation ) );
			glDrawElements( GL_LINES, _indices.size(), GL_UNSIGNED_INT, 0 ); /*launching pipeline*/
			glBindVertexArray( 0 );  /*debind VAO*/
		}

		if ( _point_mode )
		{
			glBindVertexArray( _vaoParticules ); /*bind particules VAO with the program*/
			glProgramUniformMatrix4fv( program, _uModelMatrixLoc, 1, GL_FALSE, glm::value_ptr( _transformation ) );
			glDrawElements( GL_POINTS, _indices.size(), GL_UNSIGNED_INT, 0 ); /*launching pipeline*/
			glBindVertexArray( 0 );	 /*debind VAO*/
		}
		else	//print particles with sphere
		{
			
			_list_points[ _active_particle ]->render( program );
			 if ( !_draw_all_edges )
			{
				for ( int i = 0; i < _filtered_points.size(); i++ )
					_list_points[ _filtered_points[ i ] ]->render( program );
			}
			else
			{
				glEnable( GL_CULL_FACE );
				glCullFace( GL_BACK );
				glEnable( GL_DEPTH_TEST );
				glDepthFunc( GL_LEQUAL );
				for ( int i = 0; i < _list_points.size(); i++ )
					_list_points[ i ]->render( program );
			}
		}

	}


	void DelaunayStructure::coloration() 
	{
		for (int i = 0; i < _nbparticules; i++) {
			_colors[ i ] = Vec3f( 0.5 );
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
						this->_colors[ i ] = Vec3f( 0.5 );
				}
				
				break;
			
			default: break;
		}
		if ( _list_points[ _active_particle ]->get_point_attract()->empty() )
			_indices.push_back( _active_particle );

		this->_colors[ _active_particle ] = Vec3f( 0, 1, 1 );


		for ( int i = 0; i < _list_points.size(); i++ )
			_list_points[ i ]->set_color( _colors[ i ] );
	}

}