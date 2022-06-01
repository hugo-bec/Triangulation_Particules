#ifndef __READ_FILE_HPP__
#define __READ_FILE_HPP__

#include <fstream>
#include <sstream>
#include <string>

namespace SIM_PART
{
	static inline std::string readFile( const std::string & p_filePath )
	{
		std::ifstream ifs( p_filePath, std::ifstream::in );
		if ( !ifs.is_open() )
			throw std::ios_base::failure( "Cannot open file: " + p_filePath );

		std::stringstream s;
		s << ifs.rdbuf();
		ifs.close();
		return s.str();
	}
} // namespace M3D_ISICG

#endif //__READ_FILE_HPP__
