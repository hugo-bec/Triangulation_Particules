#include "TetraFileReader.hpp"

#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <iterator>


namespace tetrasearch
{
	//=====================Nodes=======================
	void TetraFileReader::readNodes( std::string fileName, std::vector<Point*>& points )
	{
		std::ifstream fileNode( fileName, std::ios::in );

		if ( fileNode )
		{
			printf( "(lecture du fichier node...) Youpi �a fonctionne ! :D \n" );

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

					points.push_back( new Point( id, cx, cy, cz ) );
				}
			}
			fileNode.close();
		}
		else printf( "Ouverture du fichier .node impossible \n" );
	}

	//======================Tetrahedrons==========================
	// prerequisite: have to be called after readNodes.
	void TetraFileReader::readTetras( std::string					fileName,
									  std::vector<Point *> &		points,
									  std::vector<Tetrahedron *> &	tetras )
	{
		std::ifstream	  fileTetra( fileName, std::ios::in );
		Tetrahedron * t;

		if ( fileTetra )
		{
			printf( "(lecture du fichier ele...) Youpi �a fonctionne ! :D \n" );
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

					points[ p1 ]->addTetrahedron( t );
					points[ p2 ]->addTetrahedron( t );
					points[ p3 ]->addTetrahedron( t );
					points[ p4 ]->addTetrahedron( t );
				}
			}
			fileTetra.close();
		}
		else
			printf( "Ouverture du fichier .node impossible \n" );
	}
} // namespace tetrasearch