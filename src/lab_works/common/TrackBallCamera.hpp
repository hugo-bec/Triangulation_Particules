#ifndef __TRACKBALLCAMERA_HPP__
#define __TRACKBALLCAMERA_HPP__

#include "BaseCamera.hpp"
#include "GL/gl3w.h"
#include "define.hpp"

namespace SIM_PART
{
	// Freefly camera.
	class TrackBallCamera : public BaseCamera
	{
	  public:
		TrackBallCamera() : BaseCamera() {}

		void moveFront( const float p_delta );
		void moveRight( const float p_delta );
		void moveUp( const float p_delta );

	  private:
		void _computeViewMatrix();
		void _updatePositionTrackball();

	  private:
		float rayon		= 5; // rayon de la sphère
		float latitude	= 0; //"l'axe y" de la sphère
		float longitude = 0; //"l'axe x" de la sphère
	};
} // namespace M3D_ISICG

#endif // __CAMERA_HPP__x