#ifndef __RANDOM_HPP__
#define __RANDOM_HPP__

#include <random>
#include "define.hpp"

namespace SIM_PART
{
	static std::random_device rd;
	static std::mt19937		  gen( rd()); // Standard mersenne_twister_engine seeded with rd()
	static std::uniform_real_distribution<float> dis( 0.f, 1.f );

	inline float getRandomFloat() { return dis( gen ); }
	inline Vec2f getRandomVec2f() { return Vec2f( getRandomFloat(), getRandomFloat() ); }
	inline Vec3f getRandomVec3f() { return Vec3f( getRandomFloat(), getRandomFloat(), getRandomFloat() ); }
} // namespace M3D_ISICG

#endif // __RANDOM_HPP__
