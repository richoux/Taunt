#include <sstream>
#include <fstream>
#include <string>
#include <iostream>
#include <iomanip>

#include "connected_component.hpp"

int main( int argc, char* argv[] )
{
	if( argc != 2 )
	{
		std::cerr << "Usage: " << argv[0] << " map_file.txt\n";
		return EXIT_FAILURE;
	}
	std::string mapfile = std::string( argv[1] );
	
	std::stringstream ss;
// std::ifstream in( "C:/Users/Flo/Documents/GitHub/Taunt/Release/map.txt" );
	std::ifstream in( mapfile );
	if( !in.is_open() )
	{
		std::cerr << "Can't open file map.txt\n";
		return EXIT_FAILURE;
	}

	char c;

	std::vector< std::vector< int > > map;
	int height = 0;
	int width = 0;

	std::cout << "Read input file...\n";
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

	std::string labeled_mapfile = mapfile;
	labeled_mapfile.replace( labeled_mapfile.begin(), labeled_mapfile.begin() + 5, "maps/taunted/" );
	labeled_mapfile.replace( labeled_mapfile.end() - 4, labeled_mapfile.end(), "_labeled.txt" );
	std::ofstream labeled_out( labeled_mapfile );
	if( !labeled_out.is_open() )
	{
		std::cerr << "Can't open file " << labeled_mapfile << "\n";
		return EXIT_FAILURE;
	}

	std::cout << "Computing zones...\n";
	taunt::connected_component cc( map );
	std::vector< std::vector< int > > labeled_map = cc.compute_cc();
	std::vector< std::vector< char > > labeled_map_char( labeled_map.size() );
	std::vector< std::vector< bool > > contour_map( labeled_map.size() );
	for( size_t i = 0; i < labeled_map.size(); ++i )
	{
		labeled_map_char[ i ] = std::vector<char>( labeled_map[ i ].size() );
		contour_map[ i ] = std::vector<bool>( labeled_map[ i ].size(), false );

		for( size_t j = 0; j < labeled_map[ i ].size(); ++j )
			if( labeled_map[ i ][ j ] == -1 )
				labeled_map_char[ i ][ j ] = '.';
			else
				if( labeled_map[ i ][ j ] >= 1000 )
					labeled_map_char[ i ][ j ] = '-';
				else
					if( labeled_map[ i ][ j ] == 0 || labeled_map[ i ][ j ] == -2 )
						labeled_map_char[ i ][ j ] = ' ';
					else
						labeled_map_char[ i ][ j ] = labeled_map[ i ][ j ] + 64;
	}

	for( int j = 0; j < height; ++j )
	{
		for( int i = 0; i < width; ++i )
			ss << labeled_map_char[ j ][ i ];
		ss << "\n";
	}

	labeled_out << ss.str();
	labeled_out.close();

	std::string contour_mapfile = mapfile;
	contour_mapfile.replace( contour_mapfile.begin(), contour_mapfile.begin() + 5, "maps/taunted/" );
	contour_mapfile.replace( contour_mapfile.end() - 4, contour_mapfile.end(), "_contour.txt" );
	std::ofstream contour_file( contour_mapfile );
	if( !contour_file.is_open() )
	{
		std::cerr << "Can't open file " << contour_mapfile << "\n";
		return EXIT_FAILURE;
	}

	std::cout << "Computing contours...\n";
	auto contours = cc.compute_contours();
	auto simplified = cc.compute_simplified_contours();
	for( size_t i = 0 ; i < contours.size(); ++i )
		std::cout	<< "Contour[" << i << "]: " << boost::geometry::dsv(contours[i]) << "\n"
			        << "Simplified[" << i << "]: " << boost::geometry::dsv(simplified[i]) << "\n\n";

	std::stringstream ssc;

	for( auto& contour : contours )
	{
		for( auto& point : contour.outer() )
			contour_map[ point.y() ][ point.x() ] = true;

		for( auto& inner : contour.inners() )
			for( auto& point : inner )
				contour_map[ point.y() ][ point.x() ] = true;
	}
	
	for( int j = 0; j < height; ++j )
	{
		for( int i = 0; i < width; ++i )
			if( contour_map[j][i] )
				ssc << 'X';
			else
				ssc << '.';
		ssc << "\n";
	}

	contour_file << ssc.str();
	contour_file.close();

	// reverse y-axis for the SVG file
	for( size_t i = 0 ; i < contours.size(); ++i )
	{
		for( auto& p : contours[i].outer() )
			p.y( -p.y() );

		for( auto& inner : contours[i].inners() )
			for( auto& p : inner )
				p.y( -p.y() );

		for( auto& p : simplified[i].outer() )
			p.y( -p.y() );

		for( auto& inner : simplified[i].inners() )
			for( auto& p : inner )
				p.y( -p.y() );		
	}
	
	std::string contour_mapfile_svg = contour_mapfile;
	contour_mapfile_svg.replace( contour_mapfile_svg.end() - 3, contour_mapfile_svg.end(), "svg" );
	std::ofstream contour_svg( contour_mapfile_svg );
	boost::geometry::svg_mapper<point> mapper( contour_svg, 10*width, 10*height );

	for( size_t i = 0; i < contours.size(); ++i )
	{
		mapper.add( contours[i] );
		mapper.add( simplified[i] );
	}

	// point center;
	for( size_t i = 0; i < contours.size(); ++i )
	{
		mapper.map( contours[i], "fill-opacity:0.5;fill:rgb(253,0,0);stroke:rgb(253,0,0);stroke-width:2");
		mapper.map( simplified[i], "fill-opacity:0.5;fill:rgb(51,51,153);stroke:rgb(0,0,0);stroke-width:5");
		// boost::geometry::centroid( simplified[i], center );
		// mapper.text( center, std::to_string( i ), "fill-opacity:1;fill:rgb(0,0,0);stroke:rgb(0,0,0);stroke-width:2;font-size:64");
	}

	return EXIT_SUCCESS;
}
