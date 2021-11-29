#include <sstream>
#include <fstream>
#include <string>
#include <iostream>
#include <iomanip>
#include <cmath>

#include "connected_component.hpp"

int main( int argc, char* argv[] )
{
	if( argc != 2 )
	{
		std::cerr << "Usage: " << argv[0] << " map_file.txt\n";
		return EXIT_FAILURE;
	}
	std::string mapfile = std::string( argv[1] );
	std::string mapfile_height = mapfile;
	mapfile_height.replace( mapfile_height.end()-4, mapfile_height.end(), "_height.txt" );
	
// std::ifstream in( "C:/Users/Flo/Documents/GitHub/Taunt/Release/map.txt" );
	std::ifstream in( mapfile );
	if( !in.is_open() )
	{
		std::cerr << "Can't open file " << mapfile << "\n";
		return EXIT_FAILURE;
	}

	std::ifstream in_height( mapfile_height );
	if( !in_height.is_open() )
	{
		std::cerr << "Can't open file " << mapfile_height << "\n";
		return EXIT_FAILURE;
	}

	std::vector< std::vector< int > > map_height;
	std::vector< std::vector< int > > map_0;
	std::vector< std::vector< int > > map_2;
	std::vector< std::vector< int > > map_4;
	std::vector< std::vector< int > > map_unbuildable;
	int height = 0;
	int width = 0;

	std::cout << "Read input file " << mapfile_height << "...\n";
	std::string line;
	char c;

	// Dealing with a SC2 map
	if( mapfile.substr( mapfile.size() - 6, 6 ) == "LE.txt" )
	{
		while( std::getline( in_height, line ) )
		{
			map_height.push_back( std::vector< int >() );
			std::istringstream iss( line );
			std::vector<std::string> words( std::istream_iterator<std::string>{ iss },
			                                std::istream_iterator<std::string>() );
			for( auto& w : words )
			{
				if( height == 0 )
					++width;

				int h = static_cast<int>( std::round( std::stof( w ) ) );
				if( h == 2 )
					h = 0;
				if( h == 3 )
					h = 2;

				std::cout << h;
					
				switch( h )
				{
				case 0: // low
					map_height[ height ].push_back( 0 );
					break;
				case 1: // low doodad
					map_height[ height ].push_back( 1 );
					break;
				case 2: // high
					map_height[ height ].push_back( 2 );
					break;
				case 3: // high doodad, should never happen
					map_height[ height ].push_back( 3 );
					break;
				case 4: // very high
					map_height[ height ].push_back( 4 );
					break;
				default: // very high doodad
					map_height[ height ].push_back( 5 );
				}
			}
			++height;
			std::cout << "\n";
		}
	}
	else  // Dealing with a BW map
	{
		while( std::getline( in_height, line ) )
		{
			map_height.push_back( std::vector< int >() );
			std::stringstream ssi( line );
			while( ssi >> c )
			{
				if( height == 0 )
					++width;

				switch( c )
				{
				case '0': // low
					map_height[ height ].push_back( 0 );
					break;
				case '1': // low doodad
					map_height[ height ].push_back( 1 );
					break;
				case '2': // high
					map_height[ height ].push_back( 2 );
					break;
				case '3': // high doodad
					map_height[ height ].push_back( 3 );
					break;
				case '4': // very high
					map_height[ height ].push_back( 4 );
					break;
				default: // very high doodad
					map_height[ height ].push_back( 5 );
				}
			}

			++height;
		}
	}
	in_height.close();

	std::cout << "Read input file " << mapfile << "...\n";
	int row = 0;
	while( std::getline( in, line ) )
	{
		int column = 0;
		map_0.push_back( std::vector< int >() );
		map_2.push_back( std::vector< int >() );
		map_4.push_back( std::vector< int >() );
		map_unbuildable.push_back( std::vector< int >() );
		std::stringstream ssi( line );
		while( ssi >> c )
		{
			switch( c )
			{
			case '1':
				map_0[ row ].push_back( 1 );
				map_2[ row ].push_back( 1 );
				map_4[ row ].push_back( 1 );
				map_unbuildable[ row ].push_back( 1 );
				break;
			case '2':
				map_0[ row ].push_back( 1 );
				map_2[ row ].push_back( 1 );
				map_4[ row ].push_back( 1 );
				map_unbuildable[ row ].push_back( 2 );
				// if( map_height[row][column] == 4 )
				// {	
				// 	map_0[ row ].push_back( 1 );
				// 	map_2[ row ].push_back( 1 );
				// 	map_4[ row ].push_back( 2 );
				// }
				// else
				// 	if( map_height[row][column] == 2 )
				// 	{
				// 			map_0[ row ].push_back( 1 );
				// 			map_2[ row ].push_back( 2 );
				// 			map_4[ row ].push_back( 1 );
				// 	}
				// 	else
				// 		if( map_height[row][column] == 0 )
				// 		{
				// 			map_0[ row ].push_back( 2 );
				// 			map_2[ row ].push_back( 1 );
				// 			map_4[ row ].push_back( 1 );
				// 		}
				// 		else
				// 		{
				// 			map_0[ row ].push_back( 1 );
				// 			map_2[ row ].push_back( 1 );
				// 			map_4[ row ].push_back( 1 );
				// 		}
				break;
			default:
				if( map_height[row][column] == 4 )
				{
					map_0[ row ].push_back( 1 );
					map_2[ row ].push_back( 1 );
					map_4[ row ].push_back( 0 );
					map_unbuildable[ row ].push_back( 1 );
				}
				else
					if( map_height[row][column] == 2 )
					{
							map_0[ row ].push_back( 1 );
							map_2[ row ].push_back( 0 );
							map_4[ row ].push_back( 1 );
							map_unbuildable[ row ].push_back( 1 );
					}
					else
						if( map_height[row][column] == 0 )
						{
							map_0[ row ].push_back( 0 );
							map_2[ row ].push_back( 1 );
							map_4[ row ].push_back( 1 );
							map_unbuildable[ row ].push_back( 1 );
						}
						else
						{
							map_0[ row ].push_back( 1 );
							map_2[ row ].push_back( 1 );
							map_4[ row ].push_back( 1 );
							map_unbuildable[ row ].push_back( 1 );
						}
			}
			++column;
		}
		++row;
	}
	in.close();

	std::cout << "Map size (w x h): " << width << "x" << height << "\n";

	std::string labeled_mapfile_0 = mapfile;
	labeled_mapfile_0.replace( labeled_mapfile_0.begin(), labeled_mapfile_0.begin() + 5, "maps/taunted/" );
	labeled_mapfile_0.replace( labeled_mapfile_0.end() - 4, labeled_mapfile_0.end(), "_0_low_labeled.txt" );
	std::ofstream labeled_out_0( labeled_mapfile_0 );
	if( !labeled_out_0.is_open() )
	{
		std::cerr << "Can't open file " << labeled_mapfile_0 << "\n";
		return EXIT_FAILURE;
	}

	std::string labeled_mapfile_2 = mapfile;
	labeled_mapfile_2.replace( labeled_mapfile_2.begin(), labeled_mapfile_2.begin() + 5, "maps/taunted/" );
	labeled_mapfile_2.replace( labeled_mapfile_2.end() - 4, labeled_mapfile_2.end(), "_2_high_labeled.txt" );
	std::ofstream labeled_out_2( labeled_mapfile_2 );
	if( !labeled_out_2.is_open() )
	{
		std::cerr << "Can't open file " << labeled_mapfile_2 << "\n";
		return EXIT_FAILURE;
	}

	std::string labeled_mapfile_4 = mapfile;
	labeled_mapfile_4.replace( labeled_mapfile_4.begin(), labeled_mapfile_4.begin() + 5, "maps/taunted/" );
	labeled_mapfile_4.replace( labeled_mapfile_4.end() - 4, labeled_mapfile_4.end(), "_4_very_high_labeled.txt" );
	std::ofstream labeled_out_4( labeled_mapfile_4 );
	if( !labeled_out_4.is_open() )
	{
		std::cerr << "Can't open file " << labeled_mapfile_4 << "\n";
		return EXIT_FAILURE;
	}

	std::string labeled_mapfile_unbuildable = mapfile;
	labeled_mapfile_unbuildable.replace( labeled_mapfile_unbuildable.begin(), labeled_mapfile_unbuildable.begin() + 5, "maps/taunted/" );
	labeled_mapfile_unbuildable.replace( labeled_mapfile_unbuildable.end() - 4, labeled_mapfile_unbuildable.end(), "_unbuildable.txt" );
	std::ofstream labeled_out_unbuildable( labeled_mapfile_unbuildable );
	if( !labeled_out_unbuildable.is_open() )
	{
		std::cerr << "Can't open file " << labeled_mapfile_unbuildable << "\n";
		return EXIT_FAILURE;
	}

	std::cout << "Computing contours...\n";
	taunt::connected_component cc_0( map_0 );
	auto simplified_0 = cc_0.compute_simplified_contours();

	taunt::connected_component cc_2( map_2 );
	auto simplified_2 = cc_2.compute_simplified_contours();

	taunt::connected_component cc_4( map_4 );
	auto simplified_4 = cc_4.compute_simplified_contours();

	taunt::connected_component cc_unbuildable( map_unbuildable );
	auto simplified_unbuildable = cc_unbuildable.compute_simplified_contours();

	std::stringstream ss_0;
	std::stringstream ss_2;
	std::stringstream ss_4;
	std::stringstream ss_unbuildable;

	auto labeled_map_char_0 = cc_0.get_map();
	for( int j = 0; j < height; ++j )
	{
		for( int i = 0; i < width; ++i )
			ss_0 << labeled_map_char_0[ j ][ i ];
		ss_0 << "\n";
	}

	labeled_out_0 << ss_0.str();
	labeled_out_0.close();

	auto labeled_map_char_2 = cc_2.get_map();
	for( int j = 0; j < height; ++j )
	{
		for( int i = 0; i < width; ++i )
			ss_2 << labeled_map_char_2[ j ][ i ];
		ss_2 << "\n";
	}

	labeled_out_2 << ss_2.str();
	labeled_out_2.close();

	auto labeled_map_char_4 = cc_4.get_map();
	for( int j = 0; j < height; ++j )
	{
		for( int i = 0; i < width; ++i )
			ss_4 << labeled_map_char_4[ j ][ i ];
		ss_4 << "\n";
	}

	labeled_out_4 << ss_4.str();
	labeled_out_4.close();

	auto labeled_map_char_unbuildable = cc_unbuildable.get_map();
	for( int j = 0; j < height; ++j )
	{
		for( int i = 0; i < width; ++i )
			ss_unbuildable << labeled_map_char_unbuildable[ j ][ i ];
		ss_unbuildable << "\n";
	}

	labeled_out_unbuildable << ss_unbuildable.str();
	labeled_out_unbuildable.close();

// std::vector< std::vector< bool > > contour_map( height );
	std::string contour_mapfile = mapfile;
	// contour_mapfile.replace( contour_mapfile.begin(), contour_mapfile.begin() + 5, "maps/taunted/" );
	// contour_mapfile.replace( contour_mapfile.end() - 4, contour_mapfile.end(), "_contour.txt" );
	// std::ofstream contour_file( contour_mapfile );
	// if( !contour_file.is_open() )
	// {
	// 	std::cerr << "Can't open file " << contour_mapfile << "\n";
	// 	return EXIT_FAILURE;
	// }

	// std::stringstream ssc;

	// for( int j = 0; j < height; ++j )
	// 	contour_map[j] = std::vector<bool>( width, false );

	// for( const auto& contour : simplified )
	// 	for( const auto& point : contour.outer() )
	// 		contour_map[ point.y() ][ point.x() ] = true;

	// for( int j = 0; j < height; ++j )
	// {
	// 	for( int i = 0; i < width; ++i )
	// 		if( contour_map[j][i] )
	// 			ssc << 'X';
	// 		else
	// 			ssc << '.';
	// 	ssc << "\n";
	// }

	// contour_file << ssc.str();
	// contour_file.close();

	// reverse y-axis for the SVG file
	for( size_t i = 0 ; i < simplified_0.size(); ++i )
	{
		for( auto& p : simplified_0[i].outer() )
			p.y( -p.y() );
		
		for( auto& inner : simplified_0[i].inners() )
			for( auto& p : inner )
				p.y( -p.y() );	
	}

	for( size_t i = 0 ; i < simplified_2.size(); ++i )
	{
		for( auto& p : simplified_2[i].outer() )
			p.y( -p.y() );
		
		for( auto& inner : simplified_2[i].inners() )
			for( auto& p : inner )
				p.y( -p.y() );	
	}

	for( size_t i = 0 ; i < simplified_4.size(); ++i )
	{
		for( auto& p : simplified_4[i].outer() )
			p.y( -p.y() );
		
		for( auto& inner : simplified_4[i].inners() )
			for( auto& p : inner )
				p.y( -p.y() );	
	}

	for( size_t i = 0 ; i < simplified_unbuildable.size(); ++i )
	{
		for( auto& p : simplified_unbuildable[i].outer() )
			p.y( -p.y() );
		
		for( auto& inner : simplified_unbuildable[i].inners() )
			for( auto& p : inner )
				p.y( -p.y() );	
	}

	// reverse x-axis for the SVG file from SC2 maps (?!),
	if( mapfile.substr( mapfile.size() - 6, 6 ) == "LE.txt" )
		for( size_t i = 0 ; i < simplified_0.size(); ++i )
		{
			for( auto& p : simplified_0[i].outer() )
				p.x( -p.x() );
			
			for( auto& inner : simplified_0[i].inners() )
				for( auto& p : inner )
					p.x( -p.x() );
		}

	if( mapfile.substr( mapfile.size() - 6, 6 ) == "LE.txt" )
		for( size_t i = 0 ; i < simplified_2.size(); ++i )
		{
			for( auto& p : simplified_2[i].outer() )
				p.x( -p.x() );
			
			for( auto& inner : simplified_2[i].inners() )
				for( auto& p : inner )
					p.x( -p.x() );
		}

	if( mapfile.substr( mapfile.size() - 6, 6 ) == "LE.txt" )
		for( size_t i = 0 ; i < simplified_4.size(); ++i )
		{
			for( auto& p : simplified_4[i].outer() )
				p.x( -p.x() );
			
			for( auto& inner : simplified_4[i].inners() )
				for( auto& p : inner )
					p.x( -p.x() );
		}

	if( mapfile.substr( mapfile.size() - 6, 6 ) == "LE.txt" )
		for( size_t i = 0 ; i < simplified_unbuildable.size(); ++i )
		{
			for( auto& p : simplified_unbuildable[i].outer() )
				p.x( -p.x() );
			
			for( auto& inner : simplified_unbuildable[i].inners() )
				for( auto& p : inner )
					p.x( -p.x() );
		}

	std::string contour_mapfile_svg = contour_mapfile;
	contour_mapfile_svg.replace( contour_mapfile_svg.begin(), contour_mapfile_svg.begin() + 5, "maps/taunted/" );
	contour_mapfile_svg.replace( contour_mapfile_svg.end() - 4, contour_mapfile_svg.end(), "_height.svg" );

	std::string contour_mapfile_svg_0 = contour_mapfile;
	contour_mapfile_svg_0.replace( contour_mapfile_svg_0.begin(), contour_mapfile_svg_0.begin() + 5, "maps/taunted/" );
	contour_mapfile_svg_0.replace( contour_mapfile_svg_0.end() - 4, contour_mapfile_svg_0.end(), "_0_low.svg" );

	std::string contour_mapfile_svg_2 = contour_mapfile;
	contour_mapfile_svg_2.replace( contour_mapfile_svg_2.begin(), contour_mapfile_svg_2.begin() + 5, "maps/taunted/" );
	contour_mapfile_svg_2.replace( contour_mapfile_svg_2.end() - 4, contour_mapfile_svg_2.end(), "_2_high.svg" );

	std::string contour_mapfile_svg_4 = contour_mapfile;
	contour_mapfile_svg_4.replace( contour_mapfile_svg_4.begin(), contour_mapfile_svg_4.begin() + 5, "maps/taunted/" );
	contour_mapfile_svg_4.replace( contour_mapfile_svg_4.end() - 4, contour_mapfile_svg_4.end(), "_4_very_high.svg" );

	std::string contour_mapfile_svg_unbuildable = contour_mapfile;
	contour_mapfile_svg_unbuildable.replace( contour_mapfile_svg_unbuildable.begin(), contour_mapfile_svg_unbuildable.begin() + 5, "maps/taunted/" );
	contour_mapfile_svg_unbuildable.replace( contour_mapfile_svg_unbuildable.end() - 4, contour_mapfile_svg_unbuildable.end(), "_unbuildable.svg" );

	std::ofstream contour_svg( contour_mapfile_svg );
	boost::geometry::svg_mapper<point> mapper( contour_svg, 9*width, 9*height );

	std::ofstream contour_svg_0( contour_mapfile_svg_0 );
	boost::geometry::svg_mapper<point> mapper_0( contour_svg_0, 9*width, 9*height );

	std::ofstream contour_svg_2( contour_mapfile_svg_2 );
	boost::geometry::svg_mapper<point> mapper_2( contour_svg_2, 9*width, 9*height );

	std::ofstream contour_svg_4( contour_mapfile_svg_4 );
	boost::geometry::svg_mapper<point> mapper_4( contour_svg_4, 9*width, 9*height );

	std::ofstream contour_svg_unbuildable( contour_mapfile_svg_unbuildable );
	boost::geometry::svg_mapper<point> mapper_unbuildable( contour_svg_unbuildable, 9*width, 9*height );

	for( size_t i = 0; i < simplified_0.size(); ++i )
	{
		mapper.add( simplified_0[i] );
		mapper_0.add( simplified_0[i] );
	}

	for( size_t i = 0; i < simplified_2.size(); ++i )
	{
		mapper.add( simplified_2[i] );
		mapper_2.add( simplified_2[i] );
	}

	for( size_t i = 0; i < simplified_4.size(); ++i )
	{
		mapper.add( simplified_4[i] );
		mapper_4.add( simplified_4[i] );
	}

	for( size_t i = 0; i < simplified_unbuildable.size(); ++i )
	{
		mapper.add( simplified_unbuildable[i] );
		mapper_unbuildable.add( simplified_unbuildable[i] );
	}

	for( size_t i = 0; i < simplified_0.size(); ++i )
	{
		mapper.map( simplified_0[i], "fill-opacity:0.5;fill:rgb(255,255,0);stroke:rgb(0,0,0);stroke-width:5");
		mapper_0.map( simplified_0[i], "fill-opacity:0.5;fill:rgb(53,255,0);stroke:rgb(0,0,0);stroke-width:5");
	}
	
	for( size_t i = 0; i < simplified_2.size(); ++i )
	{
		mapper.map( simplified_2[i], "fill-opacity:0.5;fill:rgb(255,128,0);stroke:rgb(0,0,0);stroke-width:5");
		mapper_2.map( simplified_2[i], "fill-opacity:0.5;fill:rgb(53,255,0);stroke:rgb(0,0,0);stroke-width:5");
	}
	
	for( size_t i = 0; i < simplified_4.size(); ++i )
	{
		mapper.map( simplified_4[i], "fill-opacity:0.5;fill:rgb(255,0,0);stroke:rgb(0,0,0);stroke-width:5");
		mapper_4.map( simplified_4[i], "fill-opacity:0.5;fill:rgb(53,255,0);stroke:rgb(0,0,0);stroke-width:5");
	}
	
	for( size_t i = 0; i < simplified_unbuildable.size(); ++i )
	{
		mapper.map( simplified_unbuildable[i], "fill-opacity:0.5;fill:rgb(0,0,255);stroke:rgb(0,0,0);stroke-width:5");
		mapper_unbuildable.map( simplified_unbuildable[i], "fill-opacity:0.5;fill:rgb(53,255,0);stroke:rgb(0,0,0);stroke-width:5");
	}
	
	return EXIT_SUCCESS;
}
