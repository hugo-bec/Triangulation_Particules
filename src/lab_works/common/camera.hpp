#ifndef __CAMERA_HPP__
#define __CAMERA_HPP__

#include "BaseCamera.hpp"
#include "GL/gl3w.h"
#include "define.hpp"

namespace SIM_PART
{
	// Freefly camera.
	class Camera : public BaseCamera
	{
	  public:
		Camera() : BaseCamera() {}

		void moveFront( const float p_delta );
		void moveRight( const float p_delta );
		void moveUp( const float p_delta );

	  private:
		void _computeViewMatrix();
	};
} // namespace M3D_ISICG

#endif // __CAMERA_HPP__
