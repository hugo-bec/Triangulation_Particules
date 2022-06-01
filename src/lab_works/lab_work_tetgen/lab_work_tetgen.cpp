#include "lab_work_tetgen.hpp"

#include "glm/gtc/type_ptr.hpp"
#include "imgui.h"
#include "utils/random.hpp"
#include "utils/read_file.hpp"
#include <iostream>

namespace SIM_PART
{
	const std::string LabWorkTetgen::_shaderFolder = "src/lab_works/lab_work_tetgen/shaders/";
	float			  distance_orbite2			 = 7.f;

	LabWorkTetgen::~LabWorkTetgen() { glDeleteProgram( _program ); }

	bool LabWorkTetgen::init()
	{
		std::cout << "Initializing lab work tetgen..." << std::endl;
		// Set the color used by glClear to clear the color buffer (in render()).
		glClearColor( _bgColor.x, _bgColor.y, _bgColor.z, _bgColor.w );
		glEnable( GL_DEPTH_TEST );

		//_camera = new Camera();

		if ( !_initProgram() )
			return false;

		_cube  = _createCube();
		_cube2 = _createCube();

		// Scaling du _cube en 0.5
		_cube._transformation = glm::scale( _cube._transformation, glm::vec3( 0.5 ) );
		// Scaling du _cube2 en 0.2
		_cube2._transformation = glm::scale( _cube2._transformation, glm::vec3( 0.2 ) );
		_cube2._transformation = glm::translate( _cube2._transformation, glm::vec3( distance_orbite2, 0.f, 0.f ) );
		/*_cube2._vertexColors = {
			  Vec3f( 1, 0, 0 ), Vec3f( 1, 0, 0 ), Vec3f( 1, 0, 0 ), Vec3f( 1, 0, 0 ),
			  Vec3f( 1, 0, 0 ), Vec3f( 1, 0, 0 ), Vec3f( 1, 0, 0 ), Vec3f( 1, 0, 0 ),
		};*/

		_initCamera();
		_initBuffersCube( &_cube );
		_initBuffersCube( &_cube2 );

		glUseProgram( _program );
		glProgramUniformMatrix4fv( _program, _uModelMatrixLoc, 1, GL_FALSE, glm::value_ptr( _cube._transformation ) );

		_updateViewMatrix();
		_updateProjectionMatrix();

		std::cout << "Done!" << std::endl;
		return true;
	}

	void LabWorkTetgen::animate( const float p_deltaTime )
	{
		_cube._transformation = glm::rotate( _cube._transformation, p_deltaTime, Vec3f( 0.f, 1.f, 1.f ) );

		_cube2._transformation = glm::translate( _cube2._transformation, Vec3f( -distance_orbite2, 0.0f, 0.0f ) );
		_cube2._transformation = glm::rotate( _cube2._transformation, p_deltaTime * 7, Vec3f( 0.f, 1.f, 1.f ) );
		_cube2._transformation = glm::translate( _cube2._transformation, Vec3f( distance_orbite2, 0.0f, 0.0f ) );
	}

	void LabWorkTetgen::render()
	{
		// Clear the color buffer.
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

		glBindVertexArray( _cube._vao ); /*bind VAO avec le programme*/
		glProgramUniformMatrix4fv( _program, _uModelMatrixLoc, 1, GL_FALSE, glm::value_ptr( _cube._transformation ) );
		glDrawElements( GL_TRIANGLES, _cube._indices.size(), GL_UNSIGNED_INT, 0 ); /*lancement du pipeline*/
		glBindVertexArray( 0 );													   /*debind VAO*/

		glBindVertexArray( _cube2._vao ); /*bind VAO avec le programme*/
		glProgramUniformMatrix4fv( _program, _uModelMatrixLoc, 1, GL_FALSE, glm::value_ptr( _cube2._transformation ) );
		glDrawElements( GL_TRIANGLES, _cube2._indices.size(), GL_UNSIGNED_INT, 0 ); /*lancement du pipeline*/
		glBindVertexArray( 0 );														/*debind VAO*/
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
		{
			glClearColor( _bgColor.x, _bgColor.y, _bgColor.z, _bgColor.w );
		}

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
			{
				_camera = new TrackBallCamera();
			}
			else
			{
				_camera = new Camera();
			}
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

	void LabWorkTetgen::_initBuffersCube( Mesh * cube )
	{
		/*//Vertices
		for ( Vec3f v : _cube._vertices ) { _vertices_data.push_back( v ); }
		for ( Vec3f v : _cube2._vertices ) { _vertices_data.push_back( v ); }
		//Colors
		for ( Vec3f v : _cube._vertexColors ) { _colors_data.push_back( v ); }
		for ( Vec3f v : _cube2._vertexColors ) { _colors_data.push_back( v ); }
		//Indices
		for ( unsigned int i : _cube._indices ) { _indices_data.push_back( i ); }
		for ( unsigned int i : _cube2._indices ) { _indices_data.push_back( i + _cube._vertices.size() ); }*/

		// VBO points
		glCreateBuffers( 1, &( *cube )._vboPositions );
		glNamedBufferData( ( *cube )._vboPositions,
						   ( *cube )._vertices.size() * sizeof( Vec3f ),
						   ( *cube )._vertices.data(),
						   GL_STATIC_DRAW );
		// glNamedBufferData( _cube._vboPositions, _vertices_data.size() * sizeof( Vec3f ), _vertices_data.data(),
		// GL_STATIC_DRAW );

		// VBO couleurs
		glCreateBuffers( 1, &( *cube )._vboColors );
		glNamedBufferData( ( *cube )._vboColors,
						   ( *cube )._vertexColors.size() * sizeof( Vec3f ),
						   ( *cube )._vertexColors.data(),
						   GL_STATIC_DRAW );
		// glNamedBufferData( _cube._vboColors, _colors_data.size() * sizeof( Vec3f ), _colors_data.data(),
		// GL_STATIC_DRAW );

		// EBO
		glCreateBuffers( 1, &( *cube )._ebo );
		glNamedBufferData( ( *cube )._ebo,
						   ( *cube )._indices.size() * sizeof( unsigned int ),
						   ( *cube )._indices.data(),
						   GL_STATIC_DRAW );

		// VAO
		GLuint indexVBO_points	 = 0;
		GLuint indexVBO_couleurs = 1;
		glCreateVertexArrays( 1, &( *cube )._vao );

		// liaison VAO avec VBO points
		glEnableVertexArrayAttrib( ( *cube )._vao, indexVBO_points );
		glVertexArrayAttribFormat( ( *cube )._vao,
								   indexVBO_points,
								   3 /*car Vec3f*/,
								   GL_FLOAT /*car Vec3f*/,
								   GL_FALSE,
								   0 /*aucune séparation entre les éléments*/ );
		glVertexArrayVertexBuffer(
			( *cube )._vao, indexVBO_points, ( *cube )._vboPositions, 0 /*débute à 0*/, sizeof( Vec3f ) );
		glVertexArrayAttribBinding(
			( *cube )._vao, 0, indexVBO_points ); /*connexion avec le shader (layout(location = 0))*/

		// liaisin VAO avec VBO couleurs
		glEnableVertexArrayAttrib( ( *cube )._vao, indexVBO_couleurs );
		glVertexArrayAttribFormat( ( *cube )._vao,
								   indexVBO_couleurs,
								   3 /*car Vec3f*/,
								   GL_FLOAT /*car Vec3f*/,
								   GL_FALSE,
								   0 /*aucune séparation entre les éléments*/ );
		glVertexArrayVertexBuffer( ( *cube )._vao, indexVBO_couleurs, ( *cube )._vboColors, 0, sizeof( Vec3f ) );
		glVertexArrayAttribBinding(
			( *cube )._vao, 1, indexVBO_couleurs ); /*connexion avec le shader (layout(location = 1))*/

		// liaison VAO avec l'EBO
		glVertexArrayElementBuffer( ( *cube )._vao, ( *cube )._ebo );
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

	LabWorkTetgen::Mesh LabWorkTetgen::_createCube()
	{
		// Creation du cube
		Mesh cube		   = Mesh();
		cube._vertices	   = { Vec3f( 1, 1, 1 ),  Vec3f( 1, 1, -1 ),  Vec3f( 1, -1, -1 ), Vec3f( 1, -1, 1 ),
						   Vec3f( -1, 1, 1 ), Vec3f( -1, 1, -1 ), Vec3f( -1, -1, 1 ), Vec3f( -1, -1, -1 ) };
		cube._vertexColors = {
			getRandomVec3f(), getRandomVec3f(), getRandomVec3f(), getRandomVec3f(),
			getRandomVec3f(), getRandomVec3f(), getRandomVec3f(), getRandomVec3f(),
		};
		cube._indices = { 0, 1, 2, 0, 2, 3,	  // face droite
						  0, 1, 4, 1, 4, 5,	  // face haut
						  4, 5, 6, 5, 6, 7,	  // face gauche
						  2, 3, 6, 2, 6, 7,	  // face bas
						  0, 3, 4, 3, 4, 6,	  // face avant
						  1, 2, 5, 2, 5, 7 }; // face arrire

		return cube;
	}
} // namespace SIM_PART
