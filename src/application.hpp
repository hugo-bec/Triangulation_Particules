#ifndef __APPLICATION_HPP__
#define __APPLICATION_HPP__

#include "SDL.h"
#include "common/lab_work_manager.hpp"
#include <string>

namespace SIM_PART
{
	class Application
	{
	  public:
		Application( const std::string & p_title, const int p_width, const int p_height );
		~Application();

		int run();

	  private:
		void _initWindow();
		void _initGL();
		void _initUI();

		void _handleEvents();
		void _displayUI();

		void _screenshot();

	  private:
		// Window data.
		std::string	  _title;
		int			  _width;
		int			  _height;
		SDL_Window *  _window	 = nullptr;
		SDL_GLContext _glContext = nullptr;

		// UI data.
		bool _displayLabWorkSettings = true;

		// Settings.
		bool _running	= false;
		bool _animation = true;
		bool _vSync		= true;

		LabWorkManager _labWorkManager;
	};
} // namespace M3D_ISICG

#endif // __APPLICATION_HPP__
