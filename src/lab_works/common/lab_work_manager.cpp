#include "lab_work_manager.hpp"
#include "imgui.h"
#include "lab_works/lab_work_cgal/lab_work_cgal.hpp"
#include "lab_works/lab_work_tetgen/lab_work_tetgen.hpp"

namespace SIM_PART
{
	LabWorkManager::LabWorkManager()
	{
		_current = new LabWorkCGAL();
		_type	 = TYPE::LAB_WORK_CGAL;
	}

	void LabWorkManager::drawMenu()
	{
		// Here you can add other lab works to the menu.
		if ( ImGui::MenuItem( "Triangulation CGAL" ) )
		{
			if ( _type != TYPE::LAB_WORK_CGAL ) // Change only if needed.
			{
				// Keep window size.
				const int w = _current->getWindowWidth();
				const int h = _current->getWindowHeight();
				delete _current;			 // Delete old lab work.
				_current = new LabWorkCGAL();	 // Create new lab work.
				_type	 = TYPE::LAB_WORK_CGAL; // Update type.
				_current->resize( w, h );	 // Update window size.
				_current->init();			 // Don't forget to call init().
			}
		}

		if ( ImGui::MenuItem( "Triangulation Tetgen" ) )
		{
			if ( _type != TYPE::LAB_WORK_TETGEN) // Change only if needed.
			{
				// Keep window size.
				const int w = _current->getWindowWidth();
				const int h = _current->getWindowHeight();
				delete _current;				// Delete old lab work.
				_current = new LabWorkTetgen();	// Create new lab work.
				_type	 = TYPE::LAB_WORK_TETGEN; // Update type.
				_current->resize( w, h );		// Update window size.
				_current->init();				// Don't forget to call init().
			}
		}
	}
} // namespace M3D_ISICG
