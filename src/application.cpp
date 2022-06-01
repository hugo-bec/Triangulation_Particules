#include "application.hpp"
#include "GL/gl3w.h"
#include "define.hpp"
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl.h"
#include "utils/gl_utils.hpp"
#include <chrono>
#include <iostream>

namespace SIM_PART
{
	Application::Application( const std::string & p_title, const int p_width, const int p_height ) :
		_title( p_title ), _width( p_width ), _height( p_height )
	{
		_initWindow();
		_initGL();
		_initUI();
		_labWorkManager.resize( p_width, p_height );
		if ( !_labWorkManager.init() )
			throw std::exception( "Error initaliazing _labWorkManager" );
	}

	Application::~Application()
	{
		if ( _glContext )
		{
			SDL_GL_DeleteContext( _glContext );
		}
		if ( _window )
		{
			SDL_DestroyWindow( _window );
		}
		SDL_Quit();

		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplSDL2_Shutdown();
		ImGui::DestroyContext();
	}

	int Application::run()
	{
		ImGuiIO & io = ImGui::GetIO();

		_running = true;
		while ( _running )
		{
			_handleEvents();

			if ( _animation )
				_labWorkManager.animate( io.DeltaTime );

			_labWorkManager.render();

			_displayUI();

			SDL_GL_SwapWindow( _window );
		}

		return 0;
	}

	void Application::_initWindow()
	{
		std::cout << "Initializing window..." << std::endl;

		std::cout << "-> Init SDL" << std::endl;
		if ( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_TIMER ) != 0 )
			throw std::exception( SDL_GetError() );

		SDL_GL_SetAttribute( SDL_GL_CONTEXT_FLAGS, 0 );
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, OPENGL_VERSION_MAJOR );
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, OPENGL_VERSION_MINOR );

		SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 24 );
		SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 8 );
		SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

		std::cout << "-> Create SDL window" << std::endl;
		const std::string title = _title + " - Lab work " + std::to_string( _labWorkManager.getType() );

		_window = SDL_CreateWindow(
			title.c_str(),
			SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED,
			_width,
			_height,
			SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE );

		if ( _window == nullptr )
			throw std::exception( SDL_GetError() );

		std::cout << "Done!" << std::endl;
	}

	void Application::_initGL()
	{
		std::cout << "Initializing OpenGL..." << std::endl;

		std::cout << "-> Create SDL GL context" << std::endl;
		if ( _window == nullptr )
		{
			throw std::exception( "Error creating SDL_GLContext: SDL_window is null" );
		}
		_glContext = SDL_GL_CreateContext( _window );

		if ( _glContext == nullptr )
			throw std::exception( SDL_GetError() );

		SDL_GL_SetSwapInterval( _vSync );

		std::cout << "-> Init GL loader" << std::endl;
		if ( gl3wInit() )
		{
			throw std::exception( "gl3wInit() failed" );
		}

		if ( !gl3wIsSupported( OPENGL_VERSION_MAJOR, OPENGL_VERSION_MINOR ) )
		{
			throw std::exception( "OpenGL version not supported" );
		}

#ifdef _DEBUG
		glEnable( GL_DEBUG_OUTPUT );
		glEnable( GL_DEBUG_OUTPUT_SYNCHRONOUS );
		glDebugMessageCallback( glDebugMessage, NULL );
		glDebugMessageControl( GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE );
#endif
		std::cout << "OpenGL Version : " << glGetString( GL_VERSION ) << std::endl;
		std::cout << "GLSL Version : " << glGetString( GL_SHADING_LANGUAGE_VERSION ) << std::endl;
		std::cout << "Device : " << glGetString( GL_RENDERER ) << std::endl;

		std::cout << "Done!" << std::endl;
	}

	void Application::_initUI()
	{
		std::cout << "Initializing ImGui..." << std::endl;

		if ( _window == nullptr )
		{
			throw std::exception( "Error initializing ImGui: SDL_window is null" );
		}
		if ( _glContext == nullptr )
		{
			throw std::exception( "Error initializing ImGui: SDL_GLContext is null" );
		}

		std::cout << "-> Checking ImGui version" << std::endl;
		if ( !IMGUI_CHECKVERSION() )
			throw std::exception( "ImGui check version error" );

		ImGui::CreateContext();

		// Setup controls
		ImGuiIO & io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable keyboard controls

		// Dark style
		ImGui::StyleColorsDark();

		std::cout << "-> Setup Platform/Renderer bindings" << std::endl;
		if ( !ImGui_ImplSDL2_InitForOpenGL( _window, _glContext ) )
		{
			throw std::exception( "ImGui_ImplSDL2_InitForOpenGL() failed" );
		}
		if ( !ImGui_ImplOpenGL3_Init( GLSL_VERSION ) )
		{
			throw std::exception( "ImGui_ImplOpenGL3_Init() failed" );
		}

		std::cout << "Done!" << std::endl;
	}

	void Application::_handleEvents()
	{
		// Handle events.
		SDL_Event event;
		while ( SDL_PollEvent( &event ) )
		{
			// ImGui first.
			ImGui_ImplSDL2_ProcessEvent( &event );

			// Then lab work.
			_labWorkManager.handleEvents( event );

			// Handle exit events.
			if ( event.type == SDL_QUIT
				 || ( event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE
					  && event.window.windowID == SDL_GetWindowID( _window ) )
				 || ( event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_ESCAPE ) )
			{
				_running = false;
			}
			else if ( event.type == SDL_WINDOWEVENT )
			{
				if ( event.window.event == SDL_WINDOWEVENT_RESIZED )
				{
					_width	= event.window.data1;
					_height = event.window.data2;
					_labWorkManager.resize( _width, _height );
				}
			}
		}
	}

	void Application::_displayUI()
	{
		ImGuiIO & io = ImGui::GetIO();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame( _window );
		ImGui::NewFrame();

		// =======================================================================
		// Display menu bar
		// =======================================================================
		ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 8, 8 ) );
		if ( ImGui::BeginMainMenuBar() )
		{
			// Main menu.
			if ( ImGui::BeginMenu( "File" ) )
			{
				// Exit.
				if ( ImGui::MenuItem( "Save image" ) )
				{
					_screenshot();
				}
				if ( ImGui::MenuItem( "Exit" ) )
				{
					_running = false;
				}

				ImGui::EndMenu();
			}
			if ( ImGui::BeginMenu( "Settings" ) )
			{
				// Enable/disable animation.
				if ( ImGui::Checkbox( "Animation", &_animation ) ) {}
				// Enable/disable V-Sync.
				if ( ImGui::Checkbox( "V-Sync", &_vSync ) )
				{
					SDL_GL_SetSwapInterval( _vSync );
				}

				ImGui::EndMenu();
			}
			if ( ImGui::BeginMenu( "Current lab work" ) )
			{
				_labWorkManager.drawMenu();

				ImGui::Separator();

				// Open/close current lab work settings.
				if ( _displayLabWorkSettings )
				{
					if ( ImGui::MenuItem( "Close lab work settings" ) )
					{
						_displayLabWorkSettings = false;
					}
				}
				else
				{
					if ( ImGui::MenuItem( "Open lab work settings" ) )
					{
						_displayLabWorkSettings = true;
					}
				}

				const std::string title = _title + " - Lab work " + std::to_string( _labWorkManager.getType() );
				SDL_SetWindowTitle( _window, title.c_str() );

				ImGui::EndMenu();
			}

			ImGui::Text( "FPS: %.0f", io.Framerate );

			ImGui::EndMainMenuBar();
		}
		ImGui::PopStyleVar();

		// =======================================================================
		// Display TP settings
		// =======================================================================
		if ( _displayLabWorkSettings )
		{
			_labWorkManager.displayUI();
		}

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData() );
	}

	void Application::_screenshot()
	{
		try
		{
			const long long timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
											std::chrono::system_clock::now().time_since_epoch() )
											.count();
			const std::string screenshotName = "./screenshots/screenshot" + std::to_string( timestamp ) + ".bmp";

			std::cout << "Saving screenshot: " << screenshotName << std::endl;

			unsigned int * pixels = new unsigned int[ _width * _height * 4 ]; // 4 bytes for RGBA
			glReadPixels( 0, 0, _width, _height, GL_RGBA, GL_UNSIGNED_BYTE, pixels );
			unsigned int * flippedPixels = new unsigned int[ _width * _height * 4 ]; // 4 bytes for RGBA
#pragma omp parallel for
			for ( int i = 0; i < _height; ++i )
			{
				for ( int j = 0; j < _width; ++j )
				{
					const unsigned int idPixelSrc  = j + ( _height - i - 1 ) * _width;
					const unsigned int idPixelDest = j + i * _width;
					flippedPixels[ idPixelDest ]   = pixels[ idPixelSrc ];
				}
			}

			SDL_Surface * surf = SDL_CreateRGBSurfaceFrom( flippedPixels, _width, _height, 32, _width * 4,
#if 0
															 0xFF000000,
															 0x00FF0000,
															 0x0000FF00,
															 0x000000FF
#else
															 0x000000FF,
															 0x0000FF00,
															 0x00FF0000,
															 0xFF000000
#endif
			);
			SDL_SaveBMP( surf, screenshotName.c_str() );

			SDL_FreeSurface( surf );
			delete[] pixels;
			delete[] flippedPixels;

			std::cout << "Done!" << std::endl;
		}
		catch ( const std::exception & e )
		{
			std::cerr << "Cannot save screenshot: " << e.what() << std::endl;
		}
	}
} // namespace M3D_ISICG
