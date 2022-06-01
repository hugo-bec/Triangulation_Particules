#ifndef __DEFINE_HPP__
#define __DEFINE_HPP__

#include "glm/glm.hpp"
#include "glm/gtc/constants.hpp"
#include <string>

namespace SIM_PART
{
// General settings.
#ifdef _DEBUG
	const bool VERBOSE = true;
#else
	const bool VERBOSE = false;
#endif

	// Open GL constants.
	const int  OPENGL_VERSION_MAJOR = 4;
	const int  OPENGL_VERSION_MINOR = 5;
	const char GLSL_VERSION[]		= "#version 450";

	// Scalars.
	const float PIf			 = glm::pi<float>();
	const float PI_2f		 = glm::half_pi<float>();
	const float PI_4f		 = glm::quarter_pi<float>();
	const float PI_32f		 = glm::three_over_two_pi<float>();
	const float TWO_PIf		 = glm::two_pi<float>();
	const float INV_PIf		 = glm::one_over_pi<float>();
	const float INV_2PIf	 = glm::one_over_two_pi<float>();
	const float FLT_INFINITY = std::numeric_limits<float>::infinity();

	// Vectors.
	using Vec2i = glm::ivec2;
	using Vec3i = glm::ivec3;
	using Vec4i = glm::ivec4;
	using Vec2f = glm::vec2;
	using Vec3f = glm::vec3;
	using Vec4f = glm::vec4;

	const Vec2i VEC2I_ZERO = Vec2i( 0 );
	const Vec3i VEC3I_ZERO = Vec3i( 0 );
	const Vec4i VEC4I_ZERO = Vec4i( 0 );
	const Vec2f VEC2F_ZERO = Vec2f( 0.f );
	const Vec3f VEC3F_ZERO = Vec3f( 0.f );
	const Vec4f VEC4F_ZERO = Vec4f( 0.f );

	// Matrices.
	using Mat3f = glm::mat3;
	using Mat4f = glm::mat4;

	const Mat4f MAT3F_ID = Mat3f( 1.f );
	const Mat4f MAT4F_ID = Mat4f( 1.f );

} // namespace M3D_ISICG

#endif // __DEFINE_HPP__
