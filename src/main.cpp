#include <sstream>
#include <fstream>
#include <string>
#include <iostream>
#include <iomanip>

#include "connected_component.hpp"

int main( int argc, char* argv[] )
{
  std::stringstream ss;
  std::ifstream in( "C:/Users/Flo/Documents/GitHub/Taunt/Release/map.txt" );
  if( !in.is_open() )
  {
	std::cout << "Can't open file map.txt\n";
	return EXIT_FAILURE;
  }

  char c;

  std::vector< std::vector< int > > map;
  int height = 0;
  int width = 0;

  std::cout << "read\n";
  std::string line;
  while( std::getline( in, line ) )
  {
	map.push_back( std::vector< int >() );
	std::stringstream ssi( line );
	while( ssi >> c )
	{
	  if( height == 0 )
		++width;

	  switch( c )
	  {
	    case '1':
	  	  map[ height ].push_back( -1 );
		  break;
		case '2':
		  map[ height ].push_back( -2 );
		  break;
		default:
		  map[ height ].push_back( 0 );
	  }
	}

	++height;
  }

  in.close();
  taunt::connected_component cc( map );
  std::vector< std::vector< int > > labeled_map = cc.compute_cc();
  std::vector< std::vector< char > > labeled_map_char( labeled_map.size() );
  for( size_t i = 0; i < labeled_map.size(); ++i )
  {
	labeled_map_char[ i ] = std::vector<char>( labeled_map[ i ].size() );
	for( size_t j = 0; j < labeled_map[ i ].size(); ++j )
	  if( labeled_map[ i ][ j ] == -1 )
		labeled_map_char[ i ][ j ] = ';';
	  else
		if( labeled_map[ i ][ j ] == -2 )
		  labeled_map_char[ i ][ j ] = '.';
		else
		  labeled_map_char[ i ][ j ] = labeled_map[ i ][ j ] + 64;
  }

  std::ofstream out( "C:/Users/Flo/Documents/GitHub/Taunt/Release/labeled_map.txt" );
  if( !out.is_open() )
  {
	std::cout << "Can't open file labeled_map.txt\n";
	return EXIT_FAILURE;
  }

  for( int j = 0; j < height; ++j )
  {
	for( int i = 0; i < width; ++i )
	  ss << labeled_map_char[ j ][ i ];
	ss << "\n";
  }

  //std::cout << ss.str() << std::endl;

  out << ss.str();
  out.close();

  std::ofstream contour( "C:/Users/Flo/Documents/GitHub/Taunt/Release/contour_map.txt" );
  if( !contour.is_open() )
  {
	std::cout << "Can't open file contour_map.txt\n";
	return EXIT_FAILURE;
  }

  auto contours = cc.get_contours();
  std::stringstream ssc;

  for( int j = 0; j < height; ++j )
  {
	for( int i = 0; i < width; ++i )
	  if( contours[ j ][ i ] )
		ssc << 'X';
	  else
		ssc << '.';
	ssc << "\n";
  }

  contour << ssc.str();
  contour.close();

  return EXIT_SUCCESS;
}