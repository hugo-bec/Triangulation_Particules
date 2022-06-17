#include "lab_work_tetgen.hpp"
#include <iostream>
#include <chrono>

#include "common/models/triangle_mesh_model.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "imgui.h"
#include "utils/random.hpp"
#include "utils/read_file.hpp"

#include "Point.hpp"
#include "Tetrahedron.hpp"
#include "TetraFileReader.hpp"
#include <array>

namespace SIM_PART
{
	const std::string LabWorkTetgen::_shaderFolder = "src/lab_works/lab_work_tetgen/shaders/";
	float			  distance_orbite2			 = 7.f;
	TriangleMeshModel s1;
	int				  actif_point = 0;
	bool			  mode_edges  = true;
	bool			  print_all_edges = false;
	bool			  play			  = false;

	LabWorkTetgen::~LabWorkTetgen() { glDeleteProgram( _program ); }

	bool LabWorkTetgen::init()
	{
		std::cout << "Initializing lab work tetgen..." << std::endl;
		// Set the color used by glClear to clear the color buffer (in render()).
		glClearColor( _bgColor.x, _bgColor.y, _bgColor.z, _bgColor.w );
		glEnable( GL_DEPTH_TEST );
		glEnable( GL_VERTEX_PROGRAM_POINT_SIZE );

		if ( !_initProgram() )
			return false;

		// init objects
		_cage  = CageMesh::_createCage();
		_cage._transformation = glm::scale( _cage._transformation, _dimCage );
		_cage._initBuffersCage();

		_createParticules();
		for ( int i = 0; i < _nbparticules; i++ )
		{
			_particules._colors.push_back(Vec3f( 0 ));
		}
		_colorPoint();
		_initBuffersParticules( &_particules );

		//tetrahedralize_particules();

		s1.load( "spherebg", "./data/model/icosphere3.obj" );
		s1._transformation = glm::scale( s1._transformation, Vec3f(2.f));

		//init camera
		_initCamera();

		glUseProgram( _program );
		glProgramUniformMatrix4fv( _program, _uModelMatrixLoc, 1, GL_FALSE, glm::value_ptr( _cage._transformation ) );

		_updateViewMatrix();
		_updateProjectionMatrix();

		std::cout << "Done!" << std::endl;
		return true;
	}

	void LabWorkTetgen::init_particules(tetgenio* in) 
	{
		for ( int i = 0; i < _nbparticules; i++ )
		{
			_particules._positions.push_back(
				Vec3f( getRandomFloat() * _dimCage.x, getRandomFloat() * _dimCage.y, getRandomFloat() * _dimCage.z ) );
		}

		in->initialize();
		in->numberofpoints = _nbparticules;
		in->pointlist	  = new REAL[ in->numberofpoints * 3 ];

		for ( int i = 0; i < _nbparticules; i++ )
		{
			in->pointlist[ 3 * i ]	  = _particules._positions[ i ].x;
			in->pointlist[ 3 * i + 1 ] = _particules._positions[ i ].y;
			in->pointlist[ 3 * i + 2 ] = _particules._positions[ i ].z;
		}
	}

	void LabWorkTetgen::tetrahedralize_particules( tetgenio* in, tetgenio* out)
	{
	

		out->initialize();
		
		out->numberofpoints = _nbparticules;
	
		out->pointlist = in->pointlist;
		std::cout << "dans tetrahedralize_particules : " << out->pointlist[ 0 ] << std::endl;

		char * param = new char[ 5 ];
		param[ 0 ]	 = '\0';
		tetrahedralize( param, in, out );

		printf( "nombre tetrahedre: %d\n", out->numberoftetrahedra );
		printf( "nombre points: %d\n", out->numberofpoints );
		
	}

	void LabWorkTetgen::animate( const float p_deltaTime )
	{
		if ( play ) 
		{
			std::vector<float> coord;
			for (int i = 0; i < (int)list_points.size(); i++) 
			{
				list_points[ i ]->bronien_mvt(0.1, 10);
				coord						= list_points[ i ]->getCoord();
				_particules._positions[ i ] = Vec3f( coord[ 0 ], coord[ 1 ], coord[ 2 ] );
				tetgenMesh.pointlist[ i * 3 ] = coord[ 0 ];
				tetgenMesh.pointlist[ i * 3 + 1 ] = coord[ 1 ];
				tetgenMesh.pointlist[ i * 3 + 2 ]	  = coord[ 2 ];

			}
			tetgenio out;
			tetrahedralize_particules( &tetgenMesh, &out );
			update_particules( &out );
			_initBuffersParticules( &_particules );
			render();
		}
	}

	void LabWorkTetgen::render()
	{
		// Clear the color buffer.
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		glPointSize( 5 ); 

		glBindVertexArray( _cage._vao ); /*bind cage VAO avec le programme*/
		glProgramUniformMatrix4fv( _program, _uModelMatrixLoc, 1, GL_FALSE, glm::value_ptr( _cage._transformation ) );
		glDrawElements( GL_LINES, _cage._segments.size(), GL_UNSIGNED_INT, 0 ); /*lancement du pipeline*/
		glBindVertexArray( 0 );													   /*debind VAO*/

		glBindVertexArray( _particules._vao ); /*bind particules VAO avec le programme*/
		glProgramUniformMatrix4fv( _program, _uModelMatrixLoc, 1, GL_FALSE, glm::value_ptr( _particules._transformation ) );
		glDrawElements( GL_POINTS, _particules._indices.size(), GL_UNSIGNED_INT, 0 ); /*lancement du pipeline*/
		glBindVertexArray( 0 );													/*debind VAO*/

		// edges 
		if ( mode_edges )
		{
			glBindVertexArray( _particules._vao ); /*bind particules VAO avec le programme*/
			glProgramUniformMatrix4fv(
				_program, _uModelMatrixLoc, 1, GL_FALSE, glm::value_ptr( _particules._transformation ) );
			glDrawElements( GL_LINES, _particules._indices.size(), GL_UNSIGNED_INT, 0 ); /*lancement du pipeline*/
			glBindVertexArray( 0 );
		}
		//s1.render( _program );
	}

	void LabWorkTetgen::handleEvents( const SDL_Event & p_event )
	{
		if ( p_event.type == SDL_KEYDOWN )
		{
			switch ( p_event.key.keysym.scancode )
			{
			case SDL_SCANCODE_W: // Front
				// std::cout << "TOUCHE W" << std::endl;
				_camera->moveFront( _cameraSpeed );
				_updateViewMatrix();
				//_camera.print();
				break;
			case SDL_SCANCODE_S: // Back
				_camera->moveFront( -_cameraSpeed );
				_updateViewMatrix();
				break;
			case SDL_SCANCODE_A: // Left
				_camera->moveRight( -_cameraSpeed );
				_updateViewMatrix();
				break;
			case SDL_SCANCODE_D: // Right
				_camera->moveRight( _cameraSpeed );
				_updateViewMatrix();
				break;
			case SDL_SCANCODE_R: // Up
				_camera->moveUp( _cameraSpeed );
				_updateViewMatrix();
				break;
			case SDL_SCANCODE_F: // Bottom
				_camera->moveUp( -_cameraSpeed );
				_updateViewMatrix();
				break;
			case SDL_SCANCODE_KP_PLUS: //arrow left
				if ( actif_point == _nbparticules - 1 )
					actif_point = 0;
				else
					actif_point++;
				_colorPoint();
				_initBuffersParticules( &_particules );
				break;
			case SDL_SCANCODE_KP_MINUS: // arrow right
				if ( actif_point == 0 )
					actif_point = _nbparticules - 1;
				else
					actif_point--;
				_colorPoint();
				_initBuffersParticules( &_particules );
				break;
			case SDL_SCANCODE_E: 
				mode_edges = !mode_edges;
				render();
				break;

			case SDL_SCANCODE_T: 
				print_all_edges = !print_all_edges;
				_colorPoint();
				_initBuffersParticules( &_particules );
				break;

			case SDL_SCANCODE_P: 
				play = !play; 


			default: break;
			}
		}

		// Rotate when left click + motion (if not on Imgui widget).
		if ( p_event.type == SDL_MOUSEMOTION && p_event.motion.state & SDL_BUTTON_LMASK
			 && !ImGui::GetIO().WantCaptureMouse )
		{
			_camera->rotate( p_event.motion.xrel * _cameraSensitivity, p_event.motion.yrel * _cameraSensitivity );
			_updateViewMatrix();
		}
	}

	void LabWorkTetgen::displayUI()
	{
		ImGui::Begin( "Settings lab work tetgen" );

		// Background.
		if ( ImGui::ColorEdit3( "Background", glm::value_ptr( _bgColor ) ) )
			glClearColor( _bgColor.x, _bgColor.y, _bgColor.z, _bgColor.w );

		// Camera.
		if ( ImGui::SliderFloat( "fovy", &_fovy, 10.f, 160.f, "%01.f" ) )
		{
			_camera->setFovy( _fovy );
			_updateProjectionMatrix();
		}
		if ( ImGui::SliderFloat( "Speed", &_cameraSpeed, 0.1f, 10.f, "%01.1f" ) )
		{
			_camera->setFovy( _fovy );
			_updateProjectionMatrix();
		}
		if ( ImGui::Checkbox( "Orthogonal view", &_orthogonal_view ) )
		{
			_camera->setOrthogonal( _orthogonal_view );
			_updateProjectionMatrix();
		}
		if ( ImGui::Checkbox( "TrackBall camera", &_trackball_camera ) )
		{
			delete ( _camera );
			if ( _trackball_camera )
				_camera = new TrackBallCamera();
			else
				_camera = new Camera();
		}


		ImGui::End();
	}

	void LabWorkTetgen::resize( const int p_width, const int p_height )
	{
		BaseLabWork::resize( p_width, p_height );
		_camera->setScreenSize( p_width, p_height );
	}

	bool LabWorkTetgen::_initProgram()
	{
		// ====================================================================
		// Shaders.
		// ====================================================================
		// Create shaders.
		const GLuint vertexShader	= glCreateShader( GL_VERTEX_SHADER );
		const GLuint fragmentShader = glCreateShader( GL_FRAGMENT_SHADER );

		// Get sources from files.
		const std::string vertexShaderSrc	= readFile( _shaderFolder + "triangle.vert" );
		const std::string fragmentShaderSrc = readFile( _shaderFolder + "triangle.frag" );

		// Convert to GLchar *
		const GLchar * vSrc = vertexShaderSrc.c_str();
		const GLchar * fSrc = fragmentShaderSrc.c_str();

		// Compile vertex shader.
		glShaderSource( vertexShader, 1, &vSrc, NULL );
		glCompileShader( vertexShader );
		// Check if compilation is ok.
		GLint compiled;
		glGetShaderiv( vertexShader, GL_COMPILE_STATUS, &compiled );
		if ( !compiled )
		{
			GLchar log[ 1024 ];
			glGetShaderInfoLog( vertexShader, sizeof( log ), NULL, log );
			glDeleteShader( vertexShader );
			glDeleteShader( fragmentShader );
			std::cerr << "Error compiling vertex shader: " << log << std::endl;
			return false;
		}

		// Compile vertex shader.
		glShaderSource( fragmentShader, 1, &fSrc, NULL );
		glCompileShader( fragmentShader );
		// Check if compilation is ok.
		glGetShaderiv( fragmentShader, GL_COMPILE_STATUS, &compiled );
		if ( compiled != GL_TRUE )
		{
			GLchar log[ 1024 ];
			glGetShaderInfoLog( fragmentShader, sizeof( log ), NULL, log );
			glDeleteShader( vertexShader );
			glDeleteShader( fragmentShader );
			std::cerr << "Error compiling fragment shader: " << log << std::endl;
			return false;
		}

		// ====================================================================
		// Program.
		// ====================================================================
		// Create program.
		_program = glCreateProgram();

		// Attach shaders.
		glAttachShader( _program, vertexShader );
		glAttachShader( _program, fragmentShader );

		// Link program.
		glLinkProgram( _program );

		// Check if link is ok.
		GLint linked;
		glGetProgramiv( _program, GL_LINK_STATUS, &linked );
		if ( !linked )
		{
			GLchar log[ 1024 ];
			glGetProgramInfoLog( _program, sizeof( log ), NULL, log );
			std::cerr << "Error linking program: " << log << std::endl;
			return false;
		}

		// Shaders are now useless.
		glDeleteShader( vertexShader );
		glDeleteShader( fragmentShader );
		// ====================================================================

		// ====================================================================
		// Get uniform locations.
		// ====================================================================
		_uModelMatrixLoc	  = glGetUniformLocation( _program, "uModelMatrix" );
		_uViewMatrixLoc		  = glGetUniformLocation( _program, "uViewMatrix" );
		_uProjectionMatrixLoc = glGetUniformLocation( _program, "uProjectionMatrix" );
		// ====================================================================

		return true;
	}

	void LabWorkTetgen::_initCamera()
	{
		_camera->setScreenSize( _windowWidth, _windowHeight );
		_camera->setPosition( Vec3f( 5, 5, 20 ) );
		_updateViewMatrix();
		_updateProjectionMatrix();
	}


	void LabWorkTetgen::_initBuffersParticules( DelaunayStructure * part )
	{
		// VBO Points
		glCreateBuffers( 1, &( *part )._vboPoints );
		glNamedBufferData( ( *part )._vboPoints,
						   ( *part )._positions.size() * sizeof( Vec3f ),
						   ( *part )._positions.data(),
						   GL_STATIC_DRAW );	//attention!

		// VBO couleurs
		glCreateBuffers( 1, &( *part )._vboColors );
		glNamedBufferData( ( *part )._vboColors,
						   ( *part )._colors.size() * sizeof( Vec3f ),
						   ( *part )._colors.data(),
						   GL_STATIC_DRAW );

		// EBO segments
		//for ( int i = 0; i < ( *part )._positions.size(); i++ ) ( *part )._indices.push_back( i );
		glCreateBuffers( 1, &( *part )._ebo );
		glNamedBufferData( ( *part )._ebo,
						   ( *part )._indices.size() * sizeof( unsigned int ),
						   ( *part )._indices.data(),
						   GL_STATIC_DRAW );

		// VAO
		GLuint indexVBO_points = 0;
		GLuint indexVBO_colors = 1;
		glCreateVertexArrays( 1, &( *part )._vao );

		// liaison VAO avec VBO Points
		glEnableVertexArrayAttrib( ( *part )._vao, indexVBO_points );
		glVertexArrayAttribFormat( ( *part )._vao,
								   indexVBO_points,
								   3 /*car Vec3f*/,
								   GL_FLOAT /*car Vec3f*/,
								   GL_FALSE /*non normalisé*/,
								   0 /*aucune sparation entre les elements*/ );

		glVertexArrayVertexBuffer(
			( *part )._vao, indexVBO_points, ( *part )._vboPoints, 0 /*dbute 0*/, sizeof( Vec3f ) );
		
		glVertexArrayAttribBinding( ( *part )._vao, 0, indexVBO_points );

		// liaisin VAO avec VBO couleurs
		glEnableVertexArrayAttrib( ( *part )._vao, indexVBO_colors );
		glVertexArrayAttribFormat( ( *part )._vao,
								   indexVBO_colors,
								   3 /*car Vec3f*/,
								   GL_FLOAT /*car Vec3f*/,
								   GL_FALSE /*non normalisé*/,
								   0 /*aucune sparation entre les lments*/ );
		glVertexArrayVertexBuffer( ( *part )._vao, indexVBO_colors, ( *part )._vboColors, 0, sizeof( Vec3f ) );
		// connexion avec le shader (layout(location = 0))
		glVertexArrayAttribBinding( ( *part )._vao, 1, indexVBO_colors );


		// liaison VAO avec l'EBO
		glVertexArrayElementBuffer( ( *part )._vao, ( *part )._ebo );
	}

	

	void LabWorkTetgen::_createParticules() 
	{ 
			
		// lecture de fichier
		/* tetrasearch::TetraFileReader::readNodes( "data/tetgen1000points.node", list_points );
		tetrasearch::TetraFileReader::readTetras( "data/tetgen1000points.ele", list_points, list_tetras );
		_nbparticules = list_points.size();*/

		std::chrono::time_point<std::chrono::system_clock> start_reading, stop_reading;
		
		// Reading tetrahedrization mesh
		start_reading = std::chrono::system_clock::now();
		tetgenio out;
		out.initialize();
		init_particules( &tetgenMesh );
		tetrahedralize_particules( &tetgenMesh, &out );
		stop_reading = std::chrono::system_clock::now();
		std::chrono::duration<double> time_reading = stop_reading - start_reading;
		std::cout << "Time reading: \t\t\t\t" << time_reading.count() << "s" << std::endl;
		update_particules( &out );

	}

	void LabWorkTetgen::update_points_tetras( tetgenio* out ) 
	{
		for ( int i = 0; i < _nbparticules; i++ )
		{
			list_points.push_back( new tetrasearch::Point(
				i, out->pointlist[ i * 3 ], out->pointlist[ i * 3 + 1 ], out->pointlist[ i * 3 + 2 ] ) );
		}

		for ( int j = 0; j < out->numberoftetrahedra; j++ )
		{
			list_tetras.push_back( new tetrasearch::Tetrahedron( j,
																 out->tetrahedronlist[ j * 4 ],
																 out->tetrahedronlist[ j * 4 + 1 ],
																 out->tetrahedronlist[ j * 4 + 2 ],
																 out->tetrahedronlist[ j * 4 + 3 ] ) );
		}

		for ( int j = 0; j < list_tetras.size(); j++ )
		{
			list_points[ list_tetras[ j ]->getPoints()[ 0 ] ]->addTetrahedron( list_tetras[ j ] );
			list_points[ list_tetras[ j ]->getPoints()[ 1 ] ]->addTetrahedron( list_tetras[ j ] );
			list_points[ list_tetras[ j ]->getPoints()[ 2 ] ]->addTetrahedron( list_tetras[ j ] );
			list_points[ list_tetras[ j ]->getPoints()[ 3 ] ]->addTetrahedron( list_tetras[ j ] );
		}
	}

	void LabWorkTetgen::compute_neighbours() 
	{
		for ( int i = 0; i < (int)list_points.size(); i++ )
		{
			list_points[ i ]->computeNeighboursV2( list_tetras );
			// if ( i % ( _nbparticules / 100 ) == 0 )
			std::cout << "compute neighbours: " << i << " / " << _nbparticules << "\r";
		}
	}
	void LabWorkTetgen::compute_attract_points() 
	{
		std::vector<int> traveled_points( _nbparticules, -1 );

		for ( int i = 0; i < (int)list_points.size(); i++ )
		{
			list_points[ i ]->computePointAttractV4( 2.f, list_points, traveled_points );
			// if ( i % ( _nbparticules / 100 ) == 0 )
			std::cout << "compute attract points: " << i << " / " << _nbparticules << "\r";
		}
	}

	void LabWorkTetgen::update_particules(tetgenio* out) 
	{
		std::chrono::time_point<std::chrono::system_clock>  start_neighbours,
			stop_neighbours, start_attract, stop_attract;
		
		update_points_tetras( out );
		
		// Computing neighbours for each points
		std::cout << "Computing neighbours from tetrahedrization..." << std::endl;
		start_neighbours = std::chrono::system_clock::now();

		compute_neighbours();
		
		std::cout << std::endl;
		stop_neighbours = std::chrono::system_clock::now();

		// Computing attract points for each points
		std::cout << "Computing attract points from tetrahedrization..." << std::endl;
		start_attract = std::chrono::system_clock::now();
		
		compute_attract_points();

		std::cout << std::endl;
		stop_attract = std::chrono::system_clock::now();

		// Assign position of the point for OpenGL
		std::vector<float> coord;
		for ( int i = 0; i < _nbparticules; i++ )
		{
			coord = list_points[ i ]->getCoord();
			_particules._positions.push_back( Vec3f( coord[ 0 ], coord[ 1 ], coord[ 2 ] ) );
		}

		// Computing and printing nb particles and times
		std::chrono::duration<double> time_neighbours = stop_neighbours - start_neighbours,
									  time_attract	  = stop_attract - start_attract;

		std::cout << "Number of particles: " << _nbparticules << std::endl;

		std::cout << "Time computing neighbours: \t\t" << time_neighbours.count() << "s" << std::endl;
		std::cout << "Time computing attracted points: \t" << time_attract.count() << "s" << std::endl;
	}

	void LabWorkTetgen::_colorPoint() 
	{
		// edges
		std::vector<int> edges, tmp, tp;
		if ( !print_all_edges )
		{
			std::vector<int> attract_actif_points = list_points[ actif_point ]->getPointAttract();
			std::vector<int> tetra_actif_points, list_tetra_tmp;
			for ( int i = 0; i < attract_actif_points.size(); i++ )
			{
				list_tetra_tmp = list_points[ attract_actif_points[ i ] ]->getTetrahedron();
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
		_particules._indices.clear();
		_particules._indices.insert( _particules._indices.end(), edges.begin(), edges.end() );

		// Change color of attracted point and center point
		std::cout << "Particule choisie : " << actif_point << std::endl;

		for ( int i = 0; i < _nbparticules; i++ )
			_particules._colors[i] =  Vec3f( 0 ) ;
		
		std::vector<int> point_attract = list_points[ actif_point ]->getPointAttract();
		for ( int i = 0; i < point_attract.size(); i++ )
			_particules._colors[ point_attract[ i ] ] = Vec3f( 1, 0, 0 );

		_particules._colors[ actif_point ] = Vec3f( 0, 1, 1 );

		

	}



	void LabWorkTetgen::_updateViewMatrix()
	{
		glProgramUniformMatrix4fv( _program, _uViewMatrixLoc, 1, GL_FALSE, glm::value_ptr( _camera->getViewMatrix() ) );
	}

	void LabWorkTetgen::_updateProjectionMatrix()
	{
		glProgramUniformMatrix4fv(
			_program, _uProjectionMatrixLoc, 1, GL_FALSE, glm::value_ptr( _camera->getProjectionMatrix() ) );
	}

} // namespace SIM_PART
