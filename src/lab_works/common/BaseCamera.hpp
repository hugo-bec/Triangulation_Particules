#ifndef __BASECAMERA_HPP__
#define __BASECAMERA_HPP__

#include "GL/gl3w.h"
#include "define.hpp"

namespace SIM_PART
{
	class BaseCamera
	{
	  public:
		BaseCamera() {}

		inline const Mat4f & getViewMatrix() const { return _viewMatrix; }
		inline const Mat4f & getProjectionMatrix() const { return _projectionMatrix; }

		void setPosition( const Vec3f & p_position );
		void setLookAt( const Vec3f & p_lookAt );
		void setFovy( const float p_fovy );
		void setOrthogonal( const bool b );

		void setScreenSize( const int p_width, const int p_height );

		virtual void moveFront( const float p_delta ) = 0;
		virtual void moveRight( const float p_delta ) = 0;
		virtual void moveUp( const float p_delta )	  = 0;
		void		 rotate( const float p_yaw, const float p_pitch );

		void print() const;

	  private:
		virtual void _computeViewMatrix() = 0;
		void		 _computeProjectionMatrix();
		void		 _updateVectors();

	  protected:
		Vec3f _position		= VEC3F_ZERO;
		Vec3f _invDirection = Vec3f( 0.f, 0.f, 1.f );  // Dw dans le cours.
		Vec3f _right		= Vec3f( -1.f, 0.f, 0.f ); // Rw dans le cours.
		Vec3f _up			= Vec3f( 0.f, 1.f, 0.f );  // Uw dans le cours.
		// Angles defining the orientation in degrees
		float _yaw	 = 90.f;
		float _pitch = 0.f;

		int	  _screenWidth	= 1280;
		int	  _screenHeight = 720;
		float _aspectRatio	= float( _screenWidth ) / _screenHeight;
		float _fovy			= 60.f;
		float _zNear		= 0.1f;
		float _zFar			= 1000.f;
		bool  _orthogonal	= false;

		Mat4f _viewMatrix		= MAT4F_ID;
		Mat4f _projectionMatrix = MAT4F_ID;
	};

} // namespace M3D_ISICG

#endif
