#include "lab_work_tetgen.hpp"
#include <iostream>
#include <chrono>

#include "common/models/triangle_mesh_model.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "imgui.h"
#include "utils/random.hpp"
#include "utils/read_file.hpp"
#include "../utils/Utils.hpp"

#include "Point.hpp"
#include "Tetrahedron.hpp"
#include "TetraFileReader.hpp"
#include "DelaunayStructure.hpp"
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
	int				  iteration		  = 3;
	std::chrono::time_point<std::chrono::system_clock> start_rend, stop_rend;

	//bool			firstTime = true;

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

		// Init Cage
		_cage  = CageMesh::_createCage();
		_cage._transformation = glm::scale( _cage._transformation, _particules._dimCage );
		_cage._initBuffersCage();

		// Init Particules
		_particules._createParticules();
		_particules._colorPoint( print_all_edges, actif_point);
		_particules._initBuffersParticules();

		//init camera
		_initCamera();

		glUseProgram( _program );
		glProgramUniformMatrix4fv( _program, _uModelMatrixLoc, 1, GL_FALSE, glm::value_ptr( _cage._transformation ) );

		_updateViewMatrix();
		_updateProjectionMatrix();

		std::cout << "Done!" << std::endl;
		start_rend = std::chrono::system_clock::now();
		return true;
	}

	void LabWorkTetgen::animate( const float p_deltaTime )
	{
	
		if ( play ) 
		{
			stop_rend = std::chrono::system_clock::now();
			Utils::print_time( "time rendering: ", start_rend, stop_rend );

			std::cout << "-------------------------------------------------------------------------------" << std::endl;
			std::chrono::time_point<std::chrono::system_clock> start_bro, stop_bro, start_tet, stop_tet, start_update,
				stop_update, start_adr, stop_adr, start_color, stop_color, start_buf, stop_buf;

			start_bro = std::chrono::system_clock::now();
			const float * coord;
			for (int i = 0; i < (int)_particules.list_points.size(); i++) 
			{
				_particules.list_points[ i ]->bronien_mvt(0.1, 10);
				coord										  = _particules.list_points[ i ]->getCoord();
				_particules._positions[ i ] = Vec3f( coord[ 0 ], coord[ 1 ], coord[ 2 ] );
				_particules.tetgenMesh.pointlist[ i * 3 ] = coord[ 0 ];
				_particules.tetgenMesh.pointlist[ i * 3 + 1 ] = coord[ 1 ];
				_particules.tetgenMesh.pointlist[ i * 3 + 2 ] = coord[ 2 ];

			}
			stop_bro = std::chrono::system_clock::now();
			Utils::print_time( "time brownian: ", start_bro, stop_bro );
			
			if ( iteration % _particules.refresh_frame == 0 )
			{
				tetgenio out;

				start_tet = std::chrono::system_clock::now();
				_particules.tetrahedralize_particules( &_particules.tetgenMesh, &out );
				stop_tet = std::chrono::system_clock::now();
				Utils::print_time( "time TETGEN tetrahedralize: ", start_tet, stop_tet );

				start_update = std::chrono::system_clock::now();
				_particules.update_particules( &out );
				stop_update = std::chrono::system_clock::now();
				Utils::print_time( "time update particles: ", start_update, stop_update );
				iteration = 0;
			}

			else
			{
				start_adr = std::chrono::system_clock::now();
				for ( int j = 0; j < _particules.list_points.size(); j++ )
					_particules.list_points[ j ]->computeAttractMethodeDoubleRayon( _particules.rayon_attract,
						_particules.list_points, _particules._traveled_point, iteration, _particules.refresh_frame );
				stop_adr = std::chrono::system_clock::now();
				Utils::print_time( "time compute attract double radius: ", start_adr, stop_adr );
			}

			_particules.list_points[ actif_point ]->computePointAttractBrut( _particules.rayon_attract,
																			 _particules.list_points );
			std::cout << "nb voisin notre methode : "
					  << _particules.list_points[ actif_point ]->getPointAttract()->size()<<std::endl;


			start_color = std::chrono::system_clock::now();
			_particules._colorPoint( print_all_edges, actif_point );
			stop_color = std::chrono::system_clock::now();
			Utils::print_time( "time coloring points and generating edges: ", start_color, stop_color );

			start_buf = std::chrono::system_clock::now();
			_particules._initBuffersParticules();
			stop_buf = std::chrono::system_clock::now();
			Utils::print_time( "time updates buffers: ", start_buf, stop_buf );
			iteration++;
		}
		start_rend = std::chrono::system_clock::now();
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
				if ( actif_point == _particules._nbparticules - 1 )
					actif_point = 0;
				else
					actif_point++;
				_particules._colorPoint( print_all_edges, actif_point );
				_particules._initBuffersParticules();
				std::cout << "Particule choisie : " << actif_point << std::endl;
				
				break;
			case SDL_SCANCODE_KP_MINUS: // arrow right
				if ( actif_point == 0 )
					actif_point = _particules._nbparticules - 1;
				else
					actif_point--;
				_particules._colorPoint( print_all_edges, actif_point );
				_particules._initBuffersParticules();
				std::cout << "Particule choisie : " << actif_point << std::endl;

				break;
			case SDL_SCANCODE_E: 
				mode_edges = !mode_edges;
				render();
				break;

			case SDL_SCANCODE_T: 
				print_all_edges = !print_all_edges;
				_particules._colorPoint( print_all_edges, actif_point );
				_particules._initBuffersParticules();
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
