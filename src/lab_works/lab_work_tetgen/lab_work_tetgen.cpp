#include "lab_work_tetgen.hpp"
#include <iostream>
#include <chrono>

#include "parameters.hpp"

#include "common/models/triangle_mesh_model.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "imgui.h"
#include "utils/random.hpp"
#include "utils/read_file.hpp"

#include "Particle.hpp"
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
	bool			  play_mode		  = false;
	int				  mode_type		  = 0;
	bool			  verbose		  = true;
	int				  iteration		  = 3;

	LabWorkTetgen::~LabWorkTetgen() { glDeleteProgram( _program ); }

	bool LabWorkTetgen::init()
	{
		std::cout << "Initializing lab work tetgen..." << std::endl;
		// Set the color used by glClear to clear the color buffer (in render()).
		glClearColor( _bgColor.x, _bgColor.y, _bgColor.z, _bgColor.w );
		glEnable( GL_DEPTH_TEST );
		glEnable( GL_VERTEX_PROGRAM_POINT_SIZE );
		if ( !_initProgram() ) return false;

		// Init Cage
		_cage.init_all( CAGE_DIM );

		// Init Particules
		create_particules( NB_PARTICULES, CAGE_DIM );
		for ( int i = 0; i < NB_INIT_FIXED_POINTS; i++ )
			_particules[ i ]->set_fix( true );

		// Init Delaunay Structure
		_dstructure.set_verbose( verbose );	//for printing all time execution 
		_dstructure.init_all( _particules );

		//init camera
		_initCamera();

		glUseProgram( _program );
		glProgramUniformMatrix4fv( _program, _uModelMatrixLoc, 1, GL_FALSE, glm::value_ptr( _cage._transformation ) );

		_updateViewMatrix();
		_updateProjectionMatrix();

		std::cout << "Done!" << std::endl;
		_chrono.set_verbose( verbose );
		_chrono.start();	// for printing time of the first rendering
		return true;
	}

	void LabWorkTetgen::animate( const float p_deltaTime ) 
	{ 
		if ( play_mode )
			for ( int i = 0; i < NB_PARTICULES; i++ )
				_particules[ i ]->apply_brownian_mvt( SPEED_PARTICULES, CAGE_DIM );

		_dstructure.update_all();
	}

	void LabWorkTetgen::render()
	{
		// Clear the color buffer.
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		glPointSize( 5 ); 

		// Cage
		_cage.render( _program, _uModelMatrixLoc );

		// Delaunay Structure
		_dstructure.render( _program, _uModelMatrixLoc );
	}


	void LabWorkTetgen::create_particules( const unsigned int nb, Vec3f cage_dim ) 
	{
		for (int i=0; i<nb; i++)
			_particules.push_back( new Particle(
				i,	getRandomFloat() * cage_dim.x, 
					getRandomFloat() * cage_dim.y, 
					getRandomFloat() * cage_dim.z ) );
	}


	void LabWorkTetgen::handleEvents( const SDL_Event & p_event )
	{
		if ( p_event.type == SDL_KEYDOWN )
		{
			switch ( p_event.key.keysym.scancode )
			{
			case SDL_SCANCODE_W: // Front
				_camera->moveFront( _cameraSpeed );
				_updateViewMatrix();
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
			case SDL_SCANCODE_KP_PLUS:
				_dstructure.set_active_particle( _dstructure._active_particle + 1 );
				std::cout << "Particule choisie : " << _dstructure._active_particle << std::endl;
				break;
			case SDL_SCANCODE_KP_MINUS:
				_dstructure.set_active_particle( _dstructure._active_particle - 1 );
				std::cout << "Particule choisie : " << _dstructure._active_particle << std::endl;
				break;
			case SDL_SCANCODE_E: 
				_dstructure.set_edges_mode( !_dstructure._edges_mode );
				break;
			case SDL_SCANCODE_T: 
				_dstructure.set_draw_all_edges( !_dstructure._draw_all_edges );
				break;
			case SDL_SCANCODE_P: 
				play_mode = !play_mode;
				_dstructure.set_play_mode( play_mode );
				break;
			//mode 0, classique avec affichage des points attractifs
			//mode 1, diffusion limited aggregation
			case SDL_SCANCODE_M: 
				_dstructure.set_type_mode();
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
