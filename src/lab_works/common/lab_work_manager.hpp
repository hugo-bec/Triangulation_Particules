#ifndef __LAB_WORK_MANAGER_HPP__
#define __LAB_WORK_MANAGER_HPP__

#include "base_lab_work.hpp"

namespace SIM_PART
{
	class LabWorkManager
	{
		// Enum to identify current lab work.
		enum class TYPE
		{
			LAB_WORK_CGAL = 1,
			LAB_WORK_TETGEN = 2
		};

	  public:
		LabWorkManager();

		~LabWorkManager() { delete _current; }

		// This method draw the content of the menu "Current lab work" (Application).
		void drawMenu();

		inline int getType() const { return static_cast<int>( _type ); }

		// Initialization.
		bool init() { return _current->init(); }
		// Animation according to p_deltaTime (time since last frame).
		void animate( const float p_deltaTime ) { _current->animate( p_deltaTime ); }
		// Handle events (keyboard, mouse).
		void handleEvents( const SDL_Event & p_event ) { _current->handleEvents( p_event ); }
		// Render one frame.
		void render() { _current->render(); }
		// Display user interface.
		void displayUI() { _current->displayUI(); }

		void resize( const int p_width, const int p_height ) { _current->resize( p_width, p_height ); }

	  private:
		TYPE		  _type;
		BaseLabWork * _current = nullptr;
	};
} // namespace M3D_ISICG

#endif // __LAB_WORK_MANAGER_HPP__
