#include "lab_work_tetgen.hpp"
#include <tetgen.h>
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


namespace SIM_PART
{
	const std::string LabWorkTetgen::_shaderFolder = "src/lab_works/lab_work_tetgen/shaders/";
	float			  distance_orbite2			 = 7.f;
	TriangleMeshModel s1;
	int				  actif_point = 0;
	bool			  mode_edges  = true;

	LabWorkTetgen::~LabWorkTetgen() { glDeleteProgram( _program ); }

	bool LabWorkTetgen::init()
	{
		std::cout << "Initializing lab work tetgen..." << std::endl;
		// Set the color used by glClear to clear the color buffer (in render()).
		glClearColor( _bgColor.x, _bgColor.y, _bgColor.z, _bgColor.w );
		glEnable( GL_DEPTH_TEST );
		glEnable( GL_VERTEX_PROGRAM_POINT_SIZE );

		//_camera = new Camera();

		if ( !_initProgram() )
			return false;

		// init objects
		_cage  = _createCage();
		_cage._transformation = glm::scale( _cage._transformation, _dimCage );
		_initBuffersCage( &_cage );

		_particules = _createParticules();
		for ( int i = 0; i < _nbparticules; i++ )
		{
			_particules._colors.push_back(Vec3f( 0 ));
		}
		_colorPoint();

		_initBuffersParticules( &_particules );

		//s1.load( "spherebg", "./data/model/icosphere3.obj" );


		//init camera
		_initCamera();

		glUseProgram( _program );
		glProgramUniformMatrix4fv( _program, _uModelMatrixLoc, 1, GL_FALSE, glm::value_ptr( _cage._transformation ) );

		_updateViewMatrix();
		_updateProjectionMatrix();

		std::cout << "Done!" << std::endl;
		return true;
	}

	void LabWorkTetgen::tetrahedralize_particules()
	{
		/* tetgenio in, out;
		in.initialize();
		out.initialize();
		in.numberofpoints = _nbparticules;
		in.pointlist	  = new REAL[ in.numberofpoints * 3 ];

		for ( int i = 0; i < _nbparticules; i++ )
		{
			in.pointlist[ 3 * i ]	  = _particules._vertices[ i ].x;
			in.pointlist[ 3 * i + 1 ] = _particules._vertices[ i ].y;
			in.pointlist[ 3 * i + 2 ] = _particules._vertices[ i ].z;
		}

		char * param = new char[ 5 ];
		param[ 0 ]	 = 'v';
		param[ 1 ]	 = 'e';
		param[ 2 ]	 = 'e';
		param[ 3 ]	 = 'Q';
		param[ 4 ]	 = '\0';*/
		//tetrahedralize( param, &in, &out );
	}

	void LabWorkTetgen::animate( const float p_deltaTime )
	{
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
		_camera->setPosition( Vec3f( 0, 1, 17 ) );
		_updateViewMatrix();
		_updateProjectionMatrix();
	}

	void LabWorkTetgen::_initBuffersCage( WireMesh * cage )
	{ 
		//VBO Points
		glCreateBuffers( 1, &( *cage )._vboPoints );
		glNamedBufferData( ( *cage )._vboPoints,
						   ( *cage )._vertices.size() * sizeof( Vec3f ),
						   ( *cage )._vertices.data(),
						   GL_STATIC_DRAW );

		//EBO segments
		glCreateBuffers( 1, &( *cage )._ebo );
		glNamedBufferData( ( *cage )._ebo,
						   ( *cage )._segments.size() * sizeof( unsigned int ),
						   ( *cage )._segments.data(),
						   GL_STATIC_DRAW );

		//VAO
		GLuint indexVBO_points = 0;
		glCreateVertexArrays( 1, &( *cage )._vao );

		// liaison VAO avec VBO Points
		glEnableVertexArrayAttrib( ( *cage )._vao, indexVBO_points );
		glVertexArrayAttribFormat( ( *cage )._vao,
								   indexVBO_points,
								   3 /*car Vec3f*/,
								   GL_FLOAT /*car Vec3f*/,
								   GL_FALSE, /*non normalisé*/
								   0 /*aucune sparation entre les elements*/ );


		glVertexArrayVertexBuffer( ( *cage )._vao, indexVBO_points, ( *cage )._vboPoints, 0 /*dbute 0*/, sizeof( Vec3f ) );
		//connexion avec le shader (layout(location = 0))
		glVertexArrayAttribBinding( ( *cage )._vao, 0, indexVBO_points ); 

		// liaison VAO avec l'EBO
		glVertexArrayElementBuffer( ( *cage )._vao, ( *cage )._ebo );
	}

	void LabWorkTetgen::_initBuffersParticules( Particules * part )
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

	LabWorkTetgen::WireMesh LabWorkTetgen::_createCage()
	{
		// Creation du cage
		WireMesh cage		   = WireMesh();
		cage._vertices	   = {	Vec3f( 1, 1, 1 ),  Vec3f( 1, 1, 0 ),  Vec3f( 1, 0, 0 ), Vec3f( 1, 0, 1 ),
								Vec3f( 0, 1, 1 ), Vec3f( 0, 1, 0 ), Vec3f( 0, 0, 0 ), Vec3f( 0, 0, 1 ) };

		cage._segments = {	0, 1, 1, 2, 2, 3, 3, 0, 
							4, 5, 5, 6, 6, 7, 7, 4, 
							0, 4, 1, 5, 2, 6, 3, 7 };
		return cage;
	}

	LabWorkTetgen::Particules LabWorkTetgen::_createParticules() 
	{ 
		std::chrono::time_point<std::chrono::system_clock> start_reading, stop_reading, start_neighbours, stop_neighbours, start_attract, stop_attract;

		Particules particules = Particules();
		std::vector<tetrasearch::Tetrahedron *> list_tetras;

		// Reading tetrahedrization mesh
		start_reading = std::chrono::system_clock::now();
		tetrasearch::TetraFileReader::readNodes( "data/tetgen1000points.1.node", list_points );
		tetrasearch::TetraFileReader::readTetras( "data/tetgen1000points.1.ele", list_points, list_tetras );
		_nbparticules = list_points.size();
		stop_reading = std::chrono::system_clock::now();


		// Computing neighbours for each points
		std::cout << "Computing neighbours from tetrahedrization..." << std::endl;
		start_neighbours = std::chrono::system_clock::now();
		for ( int i = 0; i < (int)list_points.size(); i++ )
		{
			list_points[ i ]->computeNeighbours( list_tetras );
			//if ( i % (_nbparticules / 100) == 0 ) std::cout << "compute neighbours: " << i << " / " << _nbparticules << std::endl;
		}
		stop_neighbours = std::chrono::system_clock::now();


		// Computing attract points for each points
		std::cout << "Computing attract points from tetrahedrization..." << std::endl;
		start_attract = std::chrono::system_clock::now();
		std::vector<int> traveled_points( _nbparticules, -1 );
		for ( int i = 0; i < (int)list_points.size(); i++ )
		{
			list_points[ i ]->computePointAttractV3( 3.f, list_points, traveled_points );
			//if ( i % (_nbparticules / 100) == 0 ) std::cout << "compute attract points: " << i << " / " << _nbparticules << std::endl;
		}
		stop_attract = std::chrono::system_clock::now();


		// Assign position of the point for OpenGL
		std::vector<float> coord;
		for ( int i = 0; i < _nbparticules; i++ ) 
		{
			coord = list_points[ i ]->getCoord();
			particules._positions.push_back( Vec3f( coord[ 0 ], coord[ 1 ], coord[ 2 ] ) );
		}

		//_colorPoint();

		//edges

		std::vector<unsigned int> edges;
		for (int i = 0; i < (int)list_tetras.size(); i++) 
		{
			edges.push_back( list_tetras[ i ]->getPoints()[ 0 ] );
			edges.push_back( list_tetras[ i ]->getPoints()[ 1 ] );
			edges.push_back( list_tetras[ i ]->getPoints()[ 0 ] );
			edges.push_back( list_tetras[ i ]->getPoints()[ 2 ] );
			edges.push_back( list_tetras[ i ]->getPoints()[ 0 ] );
			edges.push_back( list_tetras[ i ]->getPoints()[ 3 ] );

			edges.push_back( list_tetras[ i ]->getPoints()[ 1 ] );
			edges.push_back( list_tetras[ i ]->getPoints()[ 2 ] );
			edges.push_back( list_tetras[ i ]->getPoints()[ 1 ] );
			edges.push_back( list_tetras[ i ]->getPoints()[ 3 ] );

			edges.push_back( list_tetras[ i ]->getPoints()[ 2 ] );
			edges.push_back( list_tetras[ i ]->getPoints()[ 3 ] );
		}
		
		particules._indices.insert(particules._indices.end(), edges.begin(), edges.end() );


		// Computing and printing nb particles and times
		std::chrono::duration<double>	time_reading	= stop_reading - start_reading, 
										time_neighbours = stop_neighbours - start_neighbours,
										time_attract	= stop_attract - start_attract;

		std::cout << "Number of particles: " << _nbparticules << std::endl;
		std::cout << "Time reading: \t\t\t\t" << time_reading.count() << "s" << std::endl;
		std::cout << "Time computing neighbours: \t\t" << time_neighbours.count() << "s" << std::endl;
		std::cout << "Time computing attracted points: \t" << time_attract.count() << "s" << std::endl;



		return particules;
	}

	void LabWorkTetgen::_colorPoint() 
	{
		// Change color of attracted point and center point
		std::cout << "Particule choisie : " << actif_point << std::endl;

		for ( int i = 0; i < _nbparticules; i++ )
		{
			_particules._colors[i] =  Vec3f( 0 ) ;
		}
		
		std::vector<int> point_attract = list_points[ actif_point ]->getPointAttract();
		for ( int i = 0; i < point_attract.size(); i++ )
		{
			_particules._colors[ point_attract[ i ] ] = Vec3f( 1, 0, 0 );
		}
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
