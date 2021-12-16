#include <sstream>
#include <fstream>
#include <string>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>
#include <memory>

#include <ghost/solver.hpp>

#include "connected_component.hpp"
#include "models/filtered_separations/region_builder.hpp"
#include "models/filtered_separations/print_chokes.hpp"

#define SIMPLIFY 0 // 1.5

using namespace std::literals::chrono_literals;
using point = boost::geometry::model::d2::point_xy<int>;
using segment = boost::geometry::model::segment<point>;
using polygon = boost::geometry::model::polygon<point>;
using line = boost::geometry::model::linestring<point>;

void add_chokes( const std::vector< int >& solution, const std::vector<line>& separations, std::vector< segment >& chokes )
{
	for( size_t i = 0 ; i < solution.size() ; ++i )
		if( solution[i] == 1 )
		{
			segment seg{ separations[i][0], separations[i][1] };
			chokes.push_back( seg );
		}
}

polygon enrich( const polygon& input )
{
	polygon output;
	for( auto& inner : input.inners() )
		output.inners().push_back( inner );
	
	int perimeter = static_cast<int>( input.outer().size() );
	
	for( int i = 0 ; i < perimeter - 1 ; ++i )
	{
		boost::geometry::append( output.outer(), input.outer()[i] );

		int next = i + 1;

		auto current_point = input.outer()[i];
		auto next_point = input.outer()[next];
		bool same_x = current_point.x() == next_point.x();
		bool same_y = current_point.y() == next_point.y();
		
		if( same_x || same_y )
		{
			int distance = boost::geometry::distance( current_point, next_point );
			if( distance >= 20 )
				if( same_x )
				{
					int up = current_point.y() < next_point.y() ? 1 : -1;
					point p1( current_point.x(), current_point.y() + up*(distance/3) );
					point p2( current_point.x(), current_point.y() + up*((2*distance)/3) );
					boost::geometry::append( output.outer(), p1 );
					boost::geometry::append( output.outer(), p2 );
				}
				else // same_y
				{
					int right = current_point.x() < next_point.x() ? 1 : -1;
					point p1( current_point.x() + right*(distance/3), current_point.y() );
					point p2( current_point.x() + right*((2*distance)/3), current_point.y() );
					boost::geometry::append( output.outer(), p1 );
					boost::geometry::append( output.outer(), p2 );
				}
			else
				if( distance >= 12 )
					if( same_x )
					{
						int up = current_point.y() < next_point.y() ? 1 : -1;
						point p( current_point.x(), current_point.y() + up*(distance/2) );
						boost::geometry::append( output.outer(), p );
					}
					else // same_y
					{
						int right = current_point.x() < next_point.x() ? 1 : -1;
						point p( current_point.x() + right*(distance/2), current_point.y() );
						boost::geometry::append( output.outer(), p );
					}
		}
	}
	
	boost::geometry::correct(output);
	return output;
}


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

	std::string mapfile_resources = mapfile;
	mapfile_resources.replace( mapfile_resources.end()-4, mapfile_resources.end(), "_resources.txt" );

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

	std::ifstream in_resources( mapfile_resources );
	if( !in_resources.is_open() )
	{
		std::cerr << "Can't open file " << mapfile_resources << "\n";
		return EXIT_FAILURE;
	}

	std::vector< std::vector< int > > map_height;
	std::vector< std::vector< bool > > map_resources;
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

	std::cout << "Read input file " << mapfile_resources << "...\n";
	int row = 0;
	while( std::getline( in_resources, line ) )
	{
		map_resources.push_back( std::vector< bool >() );
		std::stringstream ssi( line );
		while( ssi >> c )
		{
			if( c == '0' ) [[likely]]
				map_resources[ row ].push_back( false );
			else
				map_resources[ row ].push_back( true );
		}
		++row;
	}
	in_resources.close();

	std::cout << "Read input file " << mapfile << "...\n";
	row = 0;
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

				if( map_resources[row][column] )
					map_unbuildable[ row ].push_back( 1 );
				else
					map_unbuildable[ row ].push_back( 2 );
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

	for( int j = 0; j < height; ++j )
		for( int i = 0; i < width; ++i )
			if( map_resources[j][i] )
			{
				if( map_height[j][i] == 4 )
					map_4[j][i] = 2;
				else
					if( map_height[j][i] == 2 )
						map_2[j][i] = 2;
					else
						if( map_height[j][i] == 0 )
							map_0[j][i] = 2;				
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

	// assigning resources to zones
	size_t number_zones_0 = simplified_0.size();
	size_t number_zones_0_2 = simplified_0.size() + simplified_2.size();
	size_t number_zones_0_2_4 = simplified_0.size() + simplified_2.size() + simplified_4.size();
	std::vector< boost::geometry::model::multi_point<point> > resources( number_zones_0_2_4 );
	std::vector< int > number_clusters_resources( number_zones_0_2_4, 0 );

	std::vector< point > resource_points;
	for( int j = 0 ; j < height ; ++j )
		for( int i = 0 ; i < width ; ++i )
			if( map_resources[j][i] )
				resource_points.emplace_back( i, j );

	for( size_t i = 0 ; i < number_zones_0 ; ++i )
		for( auto& point : resource_points )
			if( boost::geometry::covered_by( point, simplified_0[ i ] ) )
				boost::geometry::append( resources[ i ], point );
	
	for( size_t i = 0 ; i < simplified_2.size() ; ++i )
		for( auto& point : resource_points )
			if( boost::geometry::covered_by( point, simplified_2[ i ] ) )
				boost::geometry::append( resources[ number_zones_0 + i ], point );

	for( size_t i = 0 ; i < simplified_4.size() ; ++i )
		for( auto& point : resource_points )
			if( boost::geometry::covered_by( point, simplified_4[ i ] ) )
				boost::geometry::append( resources[ number_zones_0_2 + i ], point );

	std::vector< std::vector< boost::geometry::model::multi_point<point> > > clusters_on_the_map( number_zones_0_2_4 );

	int max_distance;
	if( mapfile.substr( mapfile.size() - 6, 6 ) != "LE.txt" )
		max_distance = 12; // Brood War
	else
		max_distance = 17; // StarCraft 2		
	
	for( size_t i = 0 ; i < resources.size() ; ++i )
	{
		std::vector< boost::geometry::model::multi_point<point> > clusters;
		for( auto& resource : resources[i] )
		{
			bool found_cluster = false;
			for( int j = 0 ; j < static_cast<int>( clusters.size() ) ; ++j )
			{
				point center;
				boost::geometry::centroid( clusters[j], center );
				if( boost::geometry::distance( resource, center ) <= max_distance )
				{
					found_cluster = true;
					boost::geometry::append( clusters[j], resource );
				}
			}
			
			if( !found_cluster )
			{
				boost::geometry::model::multi_point<point> new_cluster{{ resource }};
				clusters.push_back( new_cluster );
			}
		}

		for( auto& cluster : clusters )
			if( boost::geometry::num_points( cluster ) >= 7 )
				clusters_on_the_map[i].push_back( cluster );
			else
			{
				boost::geometry::model::multi_point<point> temp;
				boost::geometry::difference( resources[i], cluster, temp );
				resources[i] = temp;
			}
	}

	// merge very close clusters
	int cpt = 0;
	for( auto& clusters : clusters_on_the_map )
	{
		if( clusters.empty() )
		{
			++cpt;
			continue;
		}
		
		for( size_t i = 0 ; i < clusters.size() - 1 ; ++i )
		{
			if( boost::geometry::is_empty( clusters[i] ) )
				continue;
			
			point center_i;
			boost::geometry::centroid( clusters[i], center_i );
			for( size_t j = clusters.size() - 1 ; j > i ; --j )
			{
				if( boost::geometry::is_empty( clusters[j] ) )
					continue;
			
				point center_j;
				boost::geometry::centroid( clusters[j], center_j );

				if( boost::geometry::distance( center_i, center_j ) <= 5 )
				{
					boost::geometry::model::multi_point<point> temp;
					boost::geometry::union_( clusters[i], clusters[j], temp );
					clusters[i] = temp;
					clusters.erase( clusters.begin() + j );
				}
			}
		}
		
		number_clusters_resources[cpt] = static_cast<int>( clusters.size() );
		++cpt;
	}

	std::vector< segment > chokes;
	
	std::vector<polygon> really_simplified_0( simplified_0.size() );
	std::vector<polygon> really_simplified_2( simplified_2.size() );
	std::vector<polygon> really_simplified_4( simplified_4.size() );
	
	// Compute regions with GHOST
	for( int i = 0 ; i < static_cast<int>( simplified_0.size() ) ; ++i )
	{
		if( number_clusters_resources[i] >= 2 )
		{
			polygon temp_simplified;
			boost::geometry::simplify( simplified_0[i], temp_simplified, SIMPLIFY);
			really_simplified_0[i] = enrich( temp_simplified );
			
			double cost;
			std::vector<int> solution( really_simplified_0[i].outer().size() );
			RegionBuilder builder( number_clusters_resources[i] - 1, really_simplified_0[i], clusters_on_the_map[i] );
		
			ghost::Options options;
			options.parallel_runs = true;
			options.tabu_time_selected = builder.separation_candidates.size() / number_clusters_resources[i];
			options.print = std::make_shared<PrintChokes>( builder.separation_candidates );
			options.custom_starting_point = true;
			
			ghost::Solver solver( builder );
			if( solver.solve( cost, solution, 100ms, options ) )
				add_chokes( solution, builder.separation_candidates, chokes );
			else
			{
				std::cout << "Fail to find a solution for zone " << i << "\n";
				return EXIT_FAILURE;
			}
		}
	}

	for( int i = 0 ; i < static_cast<int>( simplified_2.size() ) ; ++i )
	{
		int index = i + number_zones_0;
		if( number_clusters_resources[index] >= 2 )
		{
			polygon temp_simplified;
			boost::geometry::simplify( simplified_2[i], temp_simplified, SIMPLIFY);
			really_simplified_2[i] = enrich( temp_simplified );

			double cost;
			std::vector<int> solution( really_simplified_2[i].outer().size() );
			RegionBuilder builder( number_clusters_resources[index] - 1, really_simplified_2[i], clusters_on_the_map[index] );
		
			ghost::Options options;
			options.parallel_runs = true;
			options.tabu_time_selected = builder.separation_candidates.size() / number_clusters_resources[index];
			options.print = std::make_shared<PrintChokes>( builder.separation_candidates );
			options.custom_starting_point = true;
		
			ghost::Solver solver( builder );
			if( solver.solve( cost, solution, 100ms, options ) )
				add_chokes( solution, builder.separation_candidates, chokes );
			else
			{
				std::cout << "Fail to find a solution for zone " << index << "\n";
				return EXIT_FAILURE;
			}
		}
	}

	for( int i = 0 ; i < static_cast<int>( simplified_4.size() ) ; ++i )
	{
		int index = i + number_zones_0_2;
		if( number_clusters_resources[index] >= 2 )
		{
			polygon temp_simplified;
			boost::geometry::simplify( simplified_4[i], temp_simplified, SIMPLIFY);
			really_simplified_4[i] = enrich( temp_simplified );

			double cost;
			std::vector<int> solution( really_simplified_4[i].outer().size() );
			RegionBuilder builder( number_clusters_resources[index] - 1, really_simplified_4[i], clusters_on_the_map[index] );
		
			ghost::Options options;
			options.parallel_runs = true;
			options.tabu_time_selected = builder.separation_candidates.size() / number_clusters_resources[index];
			options.print = std::make_shared<PrintChokes>( builder.separation_candidates );
			options.custom_starting_point = true;
		
			ghost::Solver solver( builder );
			if( solver.solve( cost, solution, 100ms, options ) )
				add_chokes( solution, builder.separation_candidates, chokes );
			else
			{
				std::cout << "Fail to find a solution for zone " << index << "\n";
				return EXIT_FAILURE;
			}
		}
	}

	std::string contour_mapfile = mapfile;

	// reverse y-axis for the SVG file
	for( size_t i = 0 ; i < simplified_0.size(); ++i )
	{
		for( auto& p : simplified_0[i].outer() )
			p.y( -p.y() );
		
		for( auto& inner : simplified_0[i].inners() )
			for( auto& p : inner )
				p.y( -p.y() );	

		for( auto& p : really_simplified_0[i].outer() )
			p.y( -p.y() );
		
		for( auto& inner : really_simplified_0[i].inners() )
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

		for( auto& p : really_simplified_2[i].outer() )
			p.y( -p.y() );
		
		for( auto& inner : really_simplified_2[i].inners() )
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

		for( auto& p : really_simplified_4[i].outer() )
			p.y( -p.y() );
		
		for( auto& inner : really_simplified_4[i].inners() )
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

	for( size_t i = 0 ; i < chokes.size(); ++i )
	{
		boost::geometry::set<0, 1>( chokes[i], - boost::geometry::get<0, 1>( chokes[i] ) );
		boost::geometry::set<1, 1>( chokes[i], - boost::geometry::get<1, 1>( chokes[i] ) );
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

			for( auto& p : really_simplified_0[i].outer() )
				p.x( -p.x() );
			
			for( auto& inner : really_simplified_0[i].inners() )
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

			for( auto& p : really_simplified_2[i].outer() )
				p.x( -p.x() );
			
			for( auto& inner : really_simplified_2[i].inners() )
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

			for( auto& p : really_simplified_4[i].outer() )
				p.x( -p.x() );
			
			for( auto& inner : really_simplified_4[i].inners() )
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

	if( mapfile.substr( mapfile.size() - 6, 6 ) == "LE.txt" )
		for( size_t i = 0 ; i < chokes.size(); ++i )
		{
			boost::geometry::set<0, 0>( chokes[i], - boost::geometry::get<0, 0>( chokes[i] ) );
			boost::geometry::set<1, 0>( chokes[i], - boost::geometry::get<1, 0>( chokes[i] ) );
		}
	
	for( size_t i = 0 ; i < chokes.size(); ++i )
	{
		std::cout << "Choke [ ( "
		          << boost::geometry::get<0, 0>( chokes[i] ) << "," << boost::geometry::get<0, 1>( chokes[i] ) << " ) , ( "
		          << boost::geometry::get<1, 0>( chokes[i] ) << "," << boost::geometry::get<1, 1>( chokes[i] ) << " ) ]\n";
	}

	std::string contour_mapfile_svg = contour_mapfile;
	contour_mapfile_svg.replace( contour_mapfile_svg.begin(), contour_mapfile_svg.begin() + 5, "maps/taunted/" );
	contour_mapfile_svg.replace( contour_mapfile_svg.end() - 4, contour_mapfile_svg.end(), "_ghosted.svg" );

	std::string contour_mapfile_svg_height = contour_mapfile;
	contour_mapfile_svg_height.replace( contour_mapfile_svg_height.begin(), contour_mapfile_svg_height.begin() + 5, "maps/taunted/" );
	contour_mapfile_svg_height.replace( contour_mapfile_svg_height.end() - 4, contour_mapfile_svg_height.end(), "_height.svg" );
	
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

	std::string contour_mapfile_svg_resources = contour_mapfile;
	contour_mapfile_svg_resources.replace( contour_mapfile_svg_resources.begin(), contour_mapfile_svg_resources.begin() + 5, "maps/taunted/" );
	contour_mapfile_svg_resources.replace( contour_mapfile_svg_resources.end() - 4, contour_mapfile_svg_resources.end(), "_resources.svg" );

	std::string contour_mapfile_svg_simple = contour_mapfile;
	contour_mapfile_svg_simple.replace( contour_mapfile_svg_simple.begin(), contour_mapfile_svg_simple.begin() + 5, "maps/taunted/" );
	contour_mapfile_svg_simple.replace( contour_mapfile_svg_simple.end() - 4, contour_mapfile_svg_simple.end(), "_simple.svg" );

	std::ofstream contour_svg( contour_mapfile_svg );
	boost::geometry::svg_mapper<point> mapper( contour_svg, 9*width, 9*height );

	std::ofstream contour_svg_height( contour_mapfile_svg_height );
	boost::geometry::svg_mapper<point> mapper_height( contour_svg_height, 9*width, 9*height );

	std::ofstream contour_svg_0( contour_mapfile_svg_0 );
	boost::geometry::svg_mapper<point> mapper_0( contour_svg_0, 9*width, 9*height );

	std::ofstream contour_svg_2( contour_mapfile_svg_2 );
	boost::geometry::svg_mapper<point> mapper_2( contour_svg_2, 9*width, 9*height );

	std::ofstream contour_svg_4( contour_mapfile_svg_4 );
	boost::geometry::svg_mapper<point> mapper_4( contour_svg_4, 9*width, 9*height );

	std::ofstream contour_svg_unbuildable( contour_mapfile_svg_unbuildable );
	boost::geometry::svg_mapper<point> mapper_unbuildable( contour_svg_unbuildable, 9*width, 9*height );

	std::ofstream contour_svg_resources( contour_mapfile_svg_resources );
	boost::geometry::svg_mapper<point> mapper_resources( contour_svg_resources, 9*width, 9*height );

	std::ofstream contour_svg_simple( contour_mapfile_svg_simple );
	boost::geometry::svg_mapper<point> mapper_simple( contour_svg_simple, 9*width, 9*height );

	for( size_t i = 0; i < simplified_0.size(); ++i )
	{
		mapper.add( simplified_0[i] );
		mapper_height.add( simplified_0[i] );
		mapper_0.add( simplified_0[i] );

		mapper_simple.add( really_simplified_0[i] );
	}

	for( size_t i = 0; i < simplified_2.size(); ++i )
	{
		mapper.add( simplified_2[i] );
		mapper_height.add( simplified_2[i] );
		mapper_2.add( simplified_2[i] );

		mapper_simple.add( really_simplified_2[i] );
	}

	for( size_t i = 0; i < simplified_4.size(); ++i )
	{
		mapper.add( simplified_4[i] );
		mapper_height.add( simplified_4[i] );
		mapper_4.add( simplified_4[i] );

		mapper_simple.add( really_simplified_4[i] );
	}

	for( size_t i = 0; i < simplified_unbuildable.size(); ++i )
	{
		mapper.add( simplified_unbuildable[i] );
		mapper_height.add( simplified_unbuildable[i] );
		mapper_unbuildable.add( simplified_unbuildable[i] );
	}

	for( size_t i = 0; i < chokes.size(); ++i )
	{
		mapper.add( chokes[i] );
		mapper_height.add( chokes[i] );
		mapper_simple.add( chokes[i] );
	}
	
	for( size_t i = 0; i < simplified_0.size(); ++i )
	{
		mapper.map( simplified_0[i], "fill-opacity:0.5;fill:rgb(255,255,0);stroke:rgb(0,0,0);stroke-width:5");
		mapper_height.map( simplified_0[i], "fill-opacity:0.5;fill:rgb(255,255,0);stroke:rgb(0,0,0);stroke-width:5");
		mapper_0.map( simplified_0[i], "fill-opacity:0.5;fill:rgb(53,255,0);stroke:rgb(0,0,0);stroke-width:5");

		mapper_simple.map( really_simplified_0[i], "fill-opacity:0.5;fill:rgb(53,255,0);stroke:rgb(0,0,0);stroke-width:5");
	}
	
	for( size_t i = 0; i < simplified_2.size(); ++i )
	{
		mapper.map( simplified_2[i], "fill-opacity:0.5;fill:rgb(180,255,0);stroke:rgb(0,0,0);stroke-width:5");
		mapper_height.map( simplified_2[i], "fill-opacity:0.5;fill:rgb(255,128,0);stroke:rgb(0,0,0);stroke-width:5");
		mapper_2.map( simplified_2[i], "fill-opacity:0.5;fill:rgb(53,255,0);stroke:rgb(0,0,0);stroke-width:5");

		mapper_simple.map( really_simplified_2[i], "fill-opacity:0.5;fill:rgb(53,255,0);stroke:rgb(0,0,0);stroke-width:5");
	}
	
	for( size_t i = 0; i < simplified_4.size(); ++i )
	{
		mapper.map( simplified_4[i], "fill-opacity:0.5;fill:rgb(0,255,0);stroke:rgb(0,0,0);stroke-width:5");
		mapper_height.map( simplified_4[i], "fill-opacity:0.5;fill:rgb(255,0,0);stroke:rgb(0,0,0);stroke-width:5");
		mapper_4.map( simplified_4[i], "fill-opacity:0.5;fill:rgb(53,255,0);stroke:rgb(0,0,0);stroke-width:5");

		mapper_simple.map( really_simplified_4[i], "fill-opacity:0.5;fill:rgb(53,255,0);stroke:rgb(0,0,0);stroke-width:5");
	}
	
	for( size_t i = 0; i < simplified_unbuildable.size(); ++i )
	{
		mapper.map( simplified_unbuildable[i], "fill-opacity:0.5;fill:rgb(0,0,255);stroke:rgb(0,0,0);stroke-width:5");
		mapper_height.map( simplified_unbuildable[i], "fill-opacity:0.5;fill:rgb(0,0,255);stroke:rgb(0,0,0);stroke-width:5");
		mapper_unbuildable.map( simplified_unbuildable[i], "fill-opacity:0.5;fill:rgb(53,255,0);stroke:rgb(0,0,0);stroke-width:5");
	}

	for( size_t i = 0; i < chokes.size(); ++i )
	{
		mapper.map( chokes[i], "fill-opacity:1;fill:rgb(255,0,0);stroke:rgb(255,0,0);stroke-width:10");
		mapper_simple.map( chokes[i], "fill-opacity:1;fill:rgb(155,55,255);stroke:rgb(155,55,255);stroke-width:10");
	}
	
	return EXIT_SUCCESS;
}
