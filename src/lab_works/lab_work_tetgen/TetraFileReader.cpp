#include "TetraFileReader.hpp"

#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <iterator>


namespace SIM_PART
{
	//=====================Nodes=======================
	void TetraFileReader::readNodes(	std::string fileName, 
										std::vector<Point *> & points, 
										TriangleMeshModel & origin_model, 
										float particle_size, float particle_speed )
	{
		std::ifstream fileNode( fileName, std::ios::in );

		if ( fileNode )
		{
			printf( "(lecture du fichier node...) Youpi ça fonctionne ! :D \n" );

			std::string line;

			getline( fileNode, line ); // skip the first line
			float cx, cy, cz;
			int	  id;

			while ( getline( fileNode, line ) )
			{
				if ( line[ 0 ] != '#' )
				{
					std::istringstream		 iss( line );
					std::vector<std::string> results( std::istream_iterator<std::string> { iss },
													  std::istream_iterator<std::string>() );

					id = stoi( results[ 0 ] );
					cx = stof( results[ 1 ] );
					cy = stof( results[ 2 ] );
					cz = stof( results[ 3 ] );

					points.push_back( new Particle( id, cx, cy, cz, origin_model, particle_size, particle_speed ) );
				}
			}
			fileNode.close();
		}
		else printf( "Ouverture du fichier .node impossible \n" );
	}

	//======================Tetrahedrons==========================
	// prerequisite: have to be called after readNodes.
	void TetraFileReader::readTetras( std::string					fileName,
									  std::vector<Point *> &	   points,
									  std::vector<Tetrahedron *> &	tetras )
	{
		std::ifstream	  fileTetra( fileName, std::ios::in );
		Tetrahedron * t;

		if ( fileTetra )
		{
			printf( "(lecture du fichier ele...) Youpi ça fonctionne ! :D \n" );
			std::string line;
			getline( fileTetra, line ); // skip the first line
			int id, p1, p2, p3, p4;

			while ( getline( fileTetra, line ) )
			{
				if ( line[ 0 ] != '#' )
				{
					std::istringstream		 iss( line );
					std::vector<std::string> results( std::istream_iterator<std::string> { iss },
													  std::istream_iterator<std::string>() );
					id = stoi( results[ 0 ] );
					p1 = stoi( results[ 1 ] );
					p2 = stoi( results[ 2 ] );
					p3 = stoi( results[ 3 ] );
					p4 = stoi( results[ 4 ] );

					t = new Tetrahedron(id, p1, p2, p3, p4 );
					tetras.push_back( t );

					points[ p1 ]->add_tetrahedron( t );
					points[ p2 ]->add_tetrahedron( t );
					points[ p3 ]->add_tetrahedron( t );
					points[ p4 ]->add_tetrahedron( t );
				}
			}
			fileTetra.close();
		}
		else
			printf( "Ouverture du fichier .node impossible \n" );
	}
} // namespace tetrasearch
