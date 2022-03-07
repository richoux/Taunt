#include <cmath>
#include <fstream>

#include <ghost/solver.hpp>

#include "terrain_analysis.hpp"
#include "connected_component.hpp"
#include "model/region_builder.hpp"

using namespace std::literals::chrono_literals;

using taunt::terrain_analysis;
using taunt::region;

#define SIMPLIFY 0.5
#define TIMEOUT 100000

/******************/
/***   Common   ***/
/******************/
void terrain_analysis::make_frontiers( const std::vector<int>& solution,
																			 const std::vector<line>& separations )
{
	for( size_t i = 0; i < solution.size(); ++i )
		if( solution[ i ] == 1 )
			_frontiers.push_back( separations[ i ] );		
}

multipolygon terrain_analysis::make_regions( const std::vector<line>& separations, const boost_polygon& polygon )
{
	multiline ml_separations;
	ml_separations.assign( separations.begin(), separations.end() );

	// bufferize separations to get a multipolygon (and thicker separations)
	multipolygon buffered_separations;
	boost::geometry::buffer( ml_separations, buffered_separations, _distance_strategy, _side_strategy, _join_strategy, _end_strategy, _point_strategy );

	// only keep parts of separations that intersect with the polygon to cut. 
	multipolygon fit_separations;
	boost::geometry::intersection( buffered_separations, polygon, fit_separations );

	// add these separations into the multipolygon of all separations (used to compute the region id of separations)
	_separation_zones.insert( _separation_zones.end(), fit_separations.begin(), fit_separations.end() );

	// cut the polygon into multi_polygons
	std::list<boost_polygon> cut_polygon;
	boost::geometry::difference( polygon, fit_separations, cut_polygon );

	multipolygon mp_regions;
	mp_regions.assign( cut_polygon.begin(), cut_polygon.end() );
	
	return mp_regions;
}

void terrain_analysis::compute_region_id( const boost_polygon& region )
{
	++_last_label;
	box region_box;
	boost::geometry::envelope( region, region_box );

	// To get around a bug of boost::geometry::covered_by: very slightly enlarge the polygon to make sure bg::covered_by is working properly
	multipolygon buffered_region;
	boost::geometry::buffer( region, buffered_region, _small_distance_strategy, _side_strategy, _join_strategy, _end_strategy, _point_strategy );

	for( double y = region_box.min_corner().y() ; y <= region_box.max_corner().y() ; ++y )
		for( double x = region_box.min_corner().x() ; x <= region_box.max_corner().x() ; ++x )
		{
			point p{ x, y };
			if( boost::geometry::covered_by( p, buffered_region ) )
				_region_id[ y ][ x ] = _last_label;
		}	
}

void terrain_analysis::compute_region_id( const multipolygon& regions )
{
	for( const auto& region : regions )
		compute_region_id( region );
}

void terrain_analysis::connect_separations_and_regions()
{
	for( auto& separation : _separations )
	{
		std::map<int, int> counters;

		box polygon_box;
		boost::geometry::envelope( separation._polygon, polygon_box );

		// To get around a bug of boost::geometry::covered_by: very slightly enlarge the polygon to make sure bg::covered_by is working properly
		multipolygon buffered_polygon;
		boost::geometry::buffer( separation._polygon, buffered_polygon, _small_distance_strategy, _side_strategy, _join_strategy, _end_strategy, _point_strategy );

		for( double y = polygon_box.min_corner().y(); y <= polygon_box.max_corner().y(); ++y )
			for( double x = polygon_box.min_corner().x(); x <= polygon_box.max_corner().x(); ++x )
				if( boost::geometry::covered_by( point{ x, y }, buffered_polygon ) )
					++counters[ _region_id[ y ][ x ] ];

		int max = 0;
		int second_max = 0;

		int max_id = 0;
		int second_max_id = 0;

		for( const auto& count : counters )
			if( max < count.second )
			{
				second_max = max;
				second_max_id = max_id;
				max = count.second;
				max_id = count.first;
			}

		if( second_max == 0 ) // happen if the first element in counters was the max
			for( const auto& count : counters )
				if( second_max < count.second && count.first != max_id )
				{
					second_max = count.second;
					second_max_id = count.first;
				}

		for( size_t r = 0; r < _regions.size(); ++r )
			if( _regions[ r ].get_id() == max_id || _regions[ r ].get_id() == second_max_id )
			{
				separation._regions.push_back( &_regions[ r ] );
				_regions[ r ]._separations.push_back( &separation );
			}
	}
}

boost_polygon terrain_analysis::enrich( const boost_polygon& input ) const
{
	boost_polygon output;
	for( auto& inner : input.inners() )
		output.inners().push_back( inner );

	int perimeter = static_cast<int>( input.outer().size() );

	for( int i = 0; i < perimeter - 1; ++i )
	{
		boost::geometry::append( output.outer(), input.outer()[ i ] );

		int next = i + 1;

		auto& current_point = input.outer()[ i ];
		auto& next_point = input.outer()[ next ];
		bool same_x = current_point.x() == next_point.x();
		bool same_y = current_point.y() == next_point.y();

		if( same_x || same_y )
		{
			int distance = boost::geometry::distance( current_point, next_point );
			if( distance >= 20 )
				if( same_x )
				{
					int up = current_point.y() < next_point.y() ? 1 : -1;
					point p1( current_point.x(), current_point.y() + up * ( distance / 3 ) );
					point p2( current_point.x(), current_point.y() + up * ( ( 2 * distance ) / 3 ) );
					boost::geometry::append( output.outer(), p1 );
					boost::geometry::append( output.outer(), p2 );
				}
				else // same_y
				{
					int right = current_point.x() < next_point.x() ? 1 : -1;
					point p1( current_point.x() + right * ( distance / 3 ), current_point.y() );
					point p2( current_point.x() + right * ( ( 2 * distance ) / 3 ), current_point.y() );
					boost::geometry::append( output.outer(), p1 );
					boost::geometry::append( output.outer(), p2 );
				}
			else
				if( distance >= 12 )
					if( same_x )
					{
						int up = current_point.y() < next_point.y() ? 1 : -1;
						point p( current_point.x(), current_point.y() + up * ( distance / 2 ) );
						boost::geometry::append( output.outer(), p );
					}
					else // same_y
					{
						int right = current_point.x() < next_point.x() ? 1 : -1;
						point p( current_point.x() + right * ( distance / 2 ), current_point.y() );
						boost::geometry::append( output.outer(), p );
					}
		}
	}

	boost::geometry::correct( output );
	return output;
}

void terrain_analysis::analyze()
{
#if defined TAUNT_BENCH
	std::chrono::duration<double, std::milli> elapsed_time( 0 );
	std::chrono::time_point<std::chrono::steady_clock> start;
#endif

#if defined SC2API
	_map_width = _bot->Observation()->GetGameInfo().width;
	_map_height = _bot->Observation()->GetGameInfo().height;
	_walkable = matrix_bool( _map_height, std::vector<bool>( _map_width, true ) );
	_buildable = matrix_bool( _map_height, std::vector<bool>( _map_width, false ) );
	_depot_buildable = matrix_bool( _map_height, std::vector<bool>( _map_width, false ) );
	_resources = matrix_bool( _map_height, std::vector<bool>( _map_width, false ) );
	_region_id = matrix_int( _map_height, std::vector<int>( _map_width, 0 ) );
	_terrain_height = matrix_int( _map_height, std::vector<int>( _map_width, 0 ) );
	_terrain_properties_level_1 = matrix_bool( _map_height, std::vector<bool>( _map_width, false ) );
	_terrain_properties_level_2 = matrix_bool( _map_height, std::vector<bool>( _map_width, false ) );
	_terrain_properties_level_3 = matrix_bool( _map_height, std::vector<bool>( _map_width, false ) );
	_terrain_unbuildable_unwalkable = matrix_int( _map_height, std::vector<int>( _map_width, 0 ) );
#endif

#if defined TAUNT_BENCH
	start = std::chrono::steady_clock::now();

	std::string mapfile = map_filename();
	mapfile.replace( mapfile.end() - 4, mapfile.end(), "_log.txt" );
	std::ofstream log( mapfile );
	if( !log.is_open() )
	{
		std::cerr << "Can't open file " << mapfile << "\n";
	}
	std::stringstream ss;
#endif

	for( int y = 0; y < _map_height; ++y )
		for( int x = 0; x < _map_width; ++x )
		{
			_buildable[ y ][ x ] = compute_buildable( x, y );
			_depot_buildable[ y ][ x ] = compute_buildable( x, y );
			_walkable[ y ][ x ] = _buildable[ y ][ x ] || compute_walkable( x, y );
			_terrain_height[ y ][ x ] = compute_terrain_height( x, y );
			switch( _terrain_height[ y ][ x ] )
			{
			case 0:
				_terrain_properties_level_1[ y ][ x ] = _buildable[ y ][ x ]; //_walkable[y][x];
				break;
			case 2:
				_terrain_properties_level_2[ y ][ x ] = _buildable[ y ][ x ]; //_walkable[y][x];
				break;
			default: // case 4, but TODO 5
				_terrain_properties_level_3[ y ][ x ] = _buildable[ y ][ x ]; //_walkable[y][x];
			}

			if( !_buildable[ y ][ x ] && _walkable[ y ][ x ] )
				_terrain_unbuildable_unwalkable[ y ][ x ] = 2;
			else
				_terrain_unbuildable_unwalkable[ y ][ x ] = 1;
		}

#if defined TAUNT_BENCH
	elapsed_time = std::chrono::steady_clock::now() - start;
	ss << "Fill matrices: " << elapsed_time.count() << "\n";
#endif

#if defined SC2API
	for( auto& resource : _bot->Observation()->GetUnits() )
	{
		if( !is_mineral( resource->unit_type ) && !is_geyser( resource->unit_type ) )
			continue;

		int width = is_mineral( resource->unit_type ) ? 2 : 3; // width 2 if mineral, 3 if geyser
		int height = is_mineral( resource->unit_type ) ? 1 : 3; // height 1 if mineral, 3 if geyser
		int tile_x = static_cast<int>( std::floor( resource->pos.x ) - ( width / 2 ) );
		int tile_y = static_cast<int>( std::floor( resource->pos.y ) - ( height / 2 ) );

		for( int y = tile_y; y < tile_y + height; ++y )
			for( int x = tile_x; x < tile_x + width; ++x )
			{
				if( !is_valid_tile( x, y ) )
					continue;

				_buildable[ y ][ x ] = false;
				_resources[ y ][ x ] = true;
				_terrain_unbuildable_unwalkable[ y ][ x ] = 1;

				switch( _terrain_height[ y ][ x ] )
				{
				case 0:
					_terrain_properties_level_1[ y ][ x ] = true;
					break;
				case 2:
					_terrain_properties_level_2[ y ][ x ] = true;
					break;
				default: // case 4, but TODO 5
					_terrain_properties_level_3[ y ][ x ] = true;
				}

				// depots can't be built within 3 tiles of any resource
				for( int ry = -3; ry <= 3; ++ry )
					for( int rx = -3; rx <= 3; ++rx )
					{
						// sc2 doesn't fill out the corners of the mineral 3x3 boxes for some reason
						if( std::abs( rx ) + std::abs( ry ) == 6 )
							continue;

						if( !is_valid_tile( x + rx, y + ry ) )
							continue;

						_depot_buildable[ y + ry ][ x + rx ] = false;
					}
			}
	}
#else
#if defined TAUNT_BENCH
	start = std::chrono::steady_clock::now();
#endif

	for( auto& resource : BWAPI::Broodwar->getStaticNeutralUnits() )
	{
		if( !resource->getType().isResourceContainer() )
			continue;

		int tile_x = resource->getTilePosition().x;
		int tile_y = resource->getTilePosition().y;

		for( int y = tile_y; y < tile_y + resource->getType().tileHeight(); ++y )
			for( int x = tile_x; x < tile_x + resource->getType().tileWidth(); ++x )
			{
				_buildable[ y ][ x ] = false;
				if( resource->getInitialResources() >= 40 )
				{
					_resources[ y ][ x ] = true;
					_terrain_unbuildable_unwalkable[ y ][ x ] = 1;
				}

				switch( _terrain_height[ y ][ x ] )
				{
				case 0:
					_terrain_properties_level_1[ y ][ x ] = true;
					break;
				case 2:
					_terrain_properties_level_2[ y ][ x ] = true;
					break;
				default: // case 4, but TODO 5
					_terrain_properties_level_3[ y ][ x ] = true;
				}

				// depots can't be built within 3 tiles of any resource
				for( int ry = -3; ry <= 3; ++ry )
					for( int rx = -3; rx <= 3; ++rx )
					{
						if( !is_valid_tile( x + rx, y + ry ) )
							continue;

						_depot_buildable[ y + ry ][ x + rx ] = false;
					}
			}
	}
#endif

#if defined TAUNT_BENCH
	elapsed_time = std::chrono::steady_clock::now() - start;
	ss << "Search for resources: " << elapsed_time.count() << std::endl;
	start = std::chrono::steady_clock::now();
#endif

	taunt::connected_component cc_level_1( _terrain_properties_level_1 );
	_simplified_cc_level_1 = cc_level_1.compute_simplified_polygons();

	taunt::connected_component cc_level_2( _terrain_properties_level_2 );
	_simplified_cc_level_2 = cc_level_2.compute_simplified_polygons();

	taunt::connected_component cc_level_3( _terrain_properties_level_3 );
	_simplified_cc_level_3 = cc_level_3.compute_simplified_polygons();

	taunt::connected_component cc_unbuildable( _terrain_unbuildable_unwalkable );
	_simplified_cc_unbuildable = cc_unbuildable.compute_simplified_polygons();

#if defined TAUNT_BENCH
	elapsed_time = std::chrono::steady_clock::now() - start;
	ss << "Compute CCs: " << elapsed_time.count() << "\n";
	start = std::chrono::steady_clock::now();
#endif

	size_t number_zones_l1 = _simplified_cc_level_1.size();
	size_t number_zones_l12 = number_zones_l1 + _simplified_cc_level_2.size();
	size_t number_zones_l123 = number_zones_l12 + _simplified_cc_level_3.size();
	std::vector< multipoint > resource_clusters( number_zones_l123 );
	std::vector< int > number_resource_clusters( number_zones_l123, 0 );

	std::vector< point > resource_points;
	for( int y = 0; y < _map_height; ++y )
		for( int x = 0; x < _map_width; ++x )
			if( _resources[ y ][ x ] )
				resource_points.emplace_back( x, y );

	for( size_t i = 0; i < number_zones_l1; ++i )
		for( auto& point : resource_points )
			if( boost::geometry::covered_by( point, _simplified_cc_level_1[ i ] ) )
				boost::geometry::append( resource_clusters[ i ], point );

	for( size_t i = 0; i < _simplified_cc_level_2.size(); ++i )
		for( auto& point : resource_points )
			if( boost::geometry::covered_by( point, _simplified_cc_level_2[ i ] ) )
				boost::geometry::append( resource_clusters[ number_zones_l1 + i ], point );

	for( size_t i = 0; i < _simplified_cc_level_3.size(); ++i )
		for( auto& point : resource_points )
			if( boost::geometry::covered_by( point, _simplified_cc_level_3[ i ] ) )
				boost::geometry::append( resource_clusters[ number_zones_l12 + i ], point );

	std::vector< std::vector< multipoint > > clusters_on_the_map( number_zones_l123 );

#if defined TAUNT_BENCH
	elapsed_time = std::chrono::steady_clock::now() - start;
	ss << "Compute clusters: " << elapsed_time.count() << "\n";
	start = std::chrono::steady_clock::now();
#endif

#ifdef SC2API
	int max_distance = 17;
#else
	int max_distance = 12;
#endif

	for( size_t i = 0; i < resource_clusters.size(); ++i )
	{
		std::vector< multipoint > clusters;
		for( auto& resource : resource_clusters[ i ] )
		{
			bool found_cluster = false;
			for( int j = 0; j < static_cast<int>( clusters.size() ); ++j )
			{
				point center;
				boost::geometry::centroid( clusters[ j ], center );
				if( boost::geometry::distance( resource, center ) <= max_distance )
				{
					found_cluster = true;
					boost::geometry::append( clusters[ j ], resource );
				}
			}

			if( !found_cluster )
			{
				multipoint new_cluster{ { resource } };
				clusters.push_back( new_cluster );
			}
		}

		for( auto& cluster : clusters )
			if( boost::geometry::num_points( cluster ) >= 7 )
				clusters_on_the_map[ i ].push_back( cluster );
			else
			{
				multipoint temp;
				boost::geometry::difference( resource_clusters[ i ], cluster, temp );
				resource_clusters[ i ] = temp;
			}
	}

#if defined TAUNT_BENCH
	elapsed_time = std::chrono::steady_clock::now() - start;
	ss << "Assign resources for each cluster: " << elapsed_time.count() << "\n";
	start = std::chrono::steady_clock::now();
#endif

	// merge very close clusters
	int cpt = 0;
	for( auto& clusters : clusters_on_the_map )
	{
		if( clusters.empty() )
		{
			++cpt;
			continue;
		}

		for( size_t i = 0; i < clusters.size() - 1; ++i )
		{
			if( boost::geometry::is_empty( clusters[ i ] ) )
				continue;

			point center_i;
			boost::geometry::centroid( clusters[ i ], center_i );
			for( size_t j = clusters.size() - 1; j > i; --j )
			{
				if( boost::geometry::is_empty( clusters[ j ] ) )
					continue;

				point center_j;
				boost::geometry::centroid( clusters[ j ], center_j );

				if( boost::geometry::distance( center_i, center_j ) <= 7 )
				{
					multipoint temp;
					boost::geometry::union_( clusters[ i ], clusters[ j ], temp );
					clusters[ i ] = temp;
					clusters.erase( clusters.begin() + j );
				}
			}
		}

		number_resource_clusters[ cpt ] = static_cast<int>( clusters.size() );
		++cpt;
	}

#if defined TAUNT_BENCH
	elapsed_time = std::chrono::steady_clock::now() - start;
	ss << "Merge clusters: " << elapsed_time.count() << "\n";
	auto start_ghost = std::chrono::steady_clock::now();
#endif

	std::vector<boost_polygon> really_simplified_level_1( _simplified_cc_level_1.size() );
	std::vector<boost_polygon> really_simplified_level_2( _simplified_cc_level_2.size() );
	std::vector<boost_polygon> really_simplified_level_3( _simplified_cc_level_3.size() );

	// Compute regions with GHOST
	for( int i = 0; i < static_cast<int>( _simplified_cc_level_1.size() ); ++i )
	{
		if( number_resource_clusters[ i ] >= 2 )
		{
			int timeout = TIMEOUT * ( number_resource_clusters[ i ] - 1 );
			auto start_inner = std::chrono::steady_clock::now();

			boost_polygon temp_simplified;
			boost::geometry::simplify( _simplified_cc_level_1[ i ], temp_simplified, SIMPLIFY );
			really_simplified_level_1[ i ] = enrich( temp_simplified );

#if defined TAUNT_BENCH
			elapsed_time = std::chrono::steady_clock::now() - start_inner;
			ss << "enrich: " << elapsed_time.count() << "\n";
#endif

			double cost;
			std::vector<int> solution( really_simplified_level_1[ i ].outer().size() );

#if defined TAUNT_BENCH
			start_inner = std::chrono::steady_clock::now();
#endif

			RegionBuilder builder( number_resource_clusters[ i ] - 1, really_simplified_level_1[ i ], clusters_on_the_map[ i ] );

#if defined TAUNT_BENCH
			elapsed_time = std::chrono::steady_clock::now() - start_inner;
			ss << "Make Builder: " << elapsed_time.count() << "\n";
#endif

			ghost::Options options;
			options.tabu_time_selected = builder.separation_candidates.size() / number_resource_clusters[ i ];
			options.custom_starting_point = true;

#if defined TAUNT_BENCH
			start_inner = std::chrono::steady_clock::now();
#endif

			ghost::Solver solver( builder );

#if defined TAUNT_BENCH
			elapsed_time = std::chrono::steady_clock::now() - start_inner;
			ss << "Make Solver: " << elapsed_time.count() << "\n";
#endif

			start_inner = std::chrono::steady_clock::now();
			bool found_solution = solver.solve( cost, solution, timeout, options );

#if defined TAUNT_BENCH
			elapsed_time = std::chrono::steady_clock::now() - start_inner;
			ss << "Run Solver: " << elapsed_time.count() << "\n";
#endif

			while( !found_solution && timeout < 10 * TIMEOUT )
			{
				timeout *= 2;
#if defined TAUNT_BENCH
				ss << "Run Solver again with new timeout=" << timeout << "\n";
#endif

				start_inner = std::chrono::steady_clock::now();
				found_solution = solver.solve( cost, solution, timeout, options );

#if defined TAUNT_BENCH
				elapsed_time = std::chrono::steady_clock::now() - start_inner;
				ss << "Run Solver: " << elapsed_time.count() << "\n";
#endif
			}

			if( found_solution )
			{
#if defined TAUNT_BENCH
				start_inner = std::chrono::steady_clock::now();
#endif

				std::vector<line> separations;

				for( size_t i = 0; i < solution.size(); ++i )
					if( solution[ i ] == 1 )
					{
						separations.push_back( builder.separation_candidates[ i ] );
						_frontiers.push_back( builder.separation_candidates[ i ] );
					}

				auto regions = make_regions( separations, really_simplified_level_1[ i ] );
				compute_region_id( regions );
#if defined TAUNT_BENCH
				elapsed_time = std::chrono::steady_clock::now() - start_inner;
				ss << "Make frontiers: " << elapsed_time.count() << "\n";
#endif
			}
#if defined TAUNT_BENCH
			else
			{
				ss << "Fail to find a solution for zone " << index << "\n";
			}
#endif
		}
		else
		{
			compute_region_id( _simplified_cc_level_1[ i ] );
		}
	}
			

#if defined TAUNT_BENCH
	elapsed_time = std::chrono::steady_clock::now() - start;
	ss << "GHOST computation low terrain: " << elapsed_time.count() << "\n";
	start = std::chrono::steady_clock::now();
#endif

	for( int i = 0; i < static_cast<int>( _simplified_cc_level_2.size() ); ++i )
	{
		int index = i + number_zones_l1;
		if( number_resource_clusters[ index ] >= 2 )
		{
			int timeout = TIMEOUT * ( number_resource_clusters[ i ] - 1 );

#if defined TAUNT_BENCH
			auto start_inner = std::chrono::steady_clock::now();
#endif

			boost_polygon temp_simplified;
			boost::geometry::simplify( _simplified_cc_level_2[ i ], temp_simplified, SIMPLIFY );
			really_simplified_level_2[ i ] = enrich( temp_simplified );

#if defined TAUNT_BENCH
			elapsed_time = std::chrono::steady_clock::now() - start_inner;
			ss << "enrich: " << elapsed_time.count() << "\n";
#endif

			double cost;
			std::vector<int> solution( really_simplified_level_2[ i ].outer().size() );

#if defined TAUNT_BENCH
			start_inner = std::chrono::steady_clock::now();
#endif

			RegionBuilder builder( number_resource_clusters[ index ] - 1, really_simplified_level_2[ i ], clusters_on_the_map[ index ] );

#if defined TAUNT_BENCH
			elapsed_time = std::chrono::steady_clock::now() - start_inner;
			ss << "Make Builder: " << elapsed_time.count() << "\n";
#endif

			ghost::Options options;
			options.tabu_time_selected = builder.separation_candidates.size() / number_resource_clusters[ index ];
			options.custom_starting_point = true;

#if defined TAUNT_BENCH
			start_inner = std::chrono::steady_clock::now();
#endif

			ghost::Solver solver( builder );

#if defined TAUNT_BENCH
			elapsed_time = std::chrono::steady_clock::now() - start_inner;
			ss << "Make Solver: " << elapsed_time.count() << "\n";
			start_inner = std::chrono::steady_clock::now();
#endif

			bool found_solution = solver.solve( cost, solution, 150ms, options );

#if defined TAUNT_BENCH
			elapsed_time = std::chrono::steady_clock::now() - start_inner;
			ss << "Run Solver: " << elapsed_time.count() << "\n";
#endif

			while( !found_solution && timeout < 10 * TIMEOUT )
			{
				timeout *= 2;

#if defined TAUNT_BENCH
				ss << "Run Solver again with new timeout=" << timeout << "\n";
				start_inner = std::chrono::steady_clock::now();
#endif

				found_solution = solver.solve( cost, solution, timeout, options );

#if defined TAUNT_BENCH
				elapsed_time = std::chrono::steady_clock::now() - start_inner;
				ss << "Run Solver: " << elapsed_time.count() << "\n";
#endif
			}

			if( found_solution )
			{
#if defined TAUNT_BENCH
				start_inner = std::chrono::steady_clock::now();
#endif

				std::vector<line> separations;

				for( size_t i = 0; i < solution.size(); ++i )
					if( solution[ i ] == 1 )
					{
						separations.push_back( builder.separation_candidates[ i ] );
						_frontiers.push_back( builder.separation_candidates[ i ] );
					}

				auto regions = make_regions( separations, really_simplified_level_2[ i ] );
				compute_region_id( regions );

#if defined TAUNT_BENCH
				elapsed_time = std::chrono::steady_clock::now() - start_inner;
				ss << "Make frontiers: " << elapsed_time.count() << "\n";
#endif
			}
#if defined TAUNT_BENCH
			else
			{
				ss << "Fail to find a solution for zone " << index << "\n";
			}
#endif
		}
		else
		{
			compute_region_id( _simplified_cc_level_2[ i ] );
		}
	}

#if defined TAUNT_BENCH
	elapsed_time = std::chrono::steady_clock::now() - start;
	ss << "GHOST computation high terrain: " << elapsed_time.count() << "\n";
	start = std::chrono::steady_clock::now();
#endif

	for( int i = 0; i < static_cast<int>( _simplified_cc_level_3.size() ); ++i )
	{
		int index = i + number_zones_l12;
		if( number_resource_clusters[ index ] >= 2 )
		{
			int timeout = TIMEOUT * ( number_resource_clusters[ i ] - 1 );

#if defined TAUNT_BENCH
			auto start_inner = std::chrono::steady_clock::now();
#endif

			boost_polygon temp_simplified;
			boost::geometry::simplify( _simplified_cc_level_3[ i ], temp_simplified, SIMPLIFY );
			really_simplified_level_3[ i ] = enrich( temp_simplified );

#if defined TAUNT_BENCH
			elapsed_time = std::chrono::steady_clock::now() - start_inner;
			ss << "enrich: " << elapsed_time.count() << "\n";
#endif

			double cost;
			std::vector<int> solution( really_simplified_level_3[ i ].outer().size() );

#if defined TAUNT_BENCH
			start_inner = std::chrono::steady_clock::now();
#endif

			RegionBuilder builder( number_resource_clusters[ index ] - 1, really_simplified_level_3[ i ], clusters_on_the_map[ index ] );

#if defined TAUNT_BENCH
			elapsed_time = std::chrono::steady_clock::now() - start_inner;
			ss << "Make Builder: " << elapsed_time.count() << "\n";
#endif

			ghost::Options options;
			options.tabu_time_selected = builder.separation_candidates.size() / number_resource_clusters[ index ];
			options.custom_starting_point = true;

#if defined TAUNT_BENCH
			start_inner = std::chrono::steady_clock::now();
#endif

			ghost::Solver solver( builder );

#if defined TAUNT_BENCH
			elapsed_time = std::chrono::steady_clock::now() - start_inner;
			ss << "Make Solver: " << elapsed_time.count() << "\n";
			start_inner = std::chrono::steady_clock::now();
#endif

			bool found_solution = solver.solve( cost, solution, 150ms, options );

#if defined TAUNT_BENCH
			elapsed_time = std::chrono::steady_clock::now() - start_inner;
			ss << "Run Solver: " << elapsed_time.count() << "\n";
#endif

			while( !found_solution && timeout < 10 * TIMEOUT )
			{
				timeout *= 2;

#if defined TAUNT_BENCH
				ss << "Run Solver again with new timeout=" << timeout << "\n";
				start_inner = std::chrono::steady_clock::now();
#endif

				found_solution = solver.solve( cost, solution, timeout, options );

#if defined TAUNT_BENCH
				elapsed_time = std::chrono::steady_clock::now() - start_inner;
				ss << "Run Solver: " << elapsed_time.count() << "\n";
#endif
			}

			if( found_solution )
			{
#if defined TAUNT_BENCH
				start_inner = std::chrono::steady_clock::now();
#endif

				std::vector<line> separations;

				for( size_t i = 0; i < solution.size(); ++i )
					if( solution[ i ] == 1 )
					{
						separations.push_back( builder.separation_candidates[ i ] );
						_frontiers.push_back( builder.separation_candidates[ i ] );
					}

				auto regions = make_regions( separations, really_simplified_level_3[ i ] );
				compute_region_id( regions );

#if defined TAUNT_BENCH
				elapsed_time = std::chrono::steady_clock::now() - start_inner;
				ss << "Make frontiers: " << elapsed_time.count() << "\n";
#endif
			}
#if defined TAUNT_BENCH
			else
			{
				ss << "Fail to find a solution for zone " << index << "\n";
			}
#endif
		}
		else
		{
			compute_region_id( _simplified_cc_level_3[ i ] );
		}
	}

	// assign region id to unbuildables zones (slopes, bridges, ...) and separations.
	multipolygon unbuildables;
	unbuildables.assign( _simplified_cc_unbuildable.begin(), _simplified_cc_unbuildable.end() );
	compute_region_id( unbuildables );
	compute_region_id( _separation_zones );

	std::string map_id = map_filename();
	map_id.replace( map_id.end() - 4, map_id.end(), "_region_id.txt" );
	std::ofstream log( map_id );
	if( !log.is_open() )
	{
		std::cerr << "Can't open file " << map_id << "\n";
	}
	std::stringstream ss;
	for( int y = 0; y < _map_height; ++y )
	{
		for( int x = 0; x < _map_width; ++x )
		{
			if( _region_id[ y ][ x ] == 0 )
				ss << ".";
			else
			{
				char c = 64 + _region_id[ y ][ x ];
				ss << c;
			}
		}
		ss << "\n";
	}
	log << ss.str();
	log.close();


#if defined TAUNT_BENCH
	elapsed_time = std::chrono::steady_clock::now() - start;
	ss << "GHOST computation very high terrain: " << elapsed_time.count() << "\n";
	elapsed_time = std::chrono::steady_clock::now() - start_ghost;
	ss << "Total GHOST computation: " << elapsed_time.count() << "\n";
	log << ss.str();
	log.close();
#endif
}

void terrain_analysis::print()
{
	std::string mapfile = map_filename();

#if defined SC2API
	// reverse x-axis for the SVG file from SC2 maps (?!),
	for( size_t i = 0; i < _simplified_cc_level_1.size(); ++i )
	{
		for( auto& p : _simplified_cc_level_1[ i ].outer() )
			p.x( -p.x() );

		for( auto& inner : _simplified_cc_level_1[ i ].inners() )
			for( auto& p : inner )
				p.x( -p.x() );
	}

	for( size_t i = 0; i < _simplified_cc_level_2.size(); ++i )
	{
		for( auto& p : _simplified_cc_level_2[ i ].outer() )
			p.x( -p.x() );

		for( auto& inner : _simplified_cc_level_2[ i ].inners() )
			for( auto& p : inner )
				p.x( -p.x() );
	}

	for( size_t i = 0; i < _simplified_cc_level_3.size(); ++i )
	{
		for( auto& p : _simplified_cc_level_3[ i ].outer() )
			p.x( -p.x() );

		for( auto& inner : _simplified_cc_level_3[ i ].inners() )
			for( auto& p : inner )
				p.x( -p.x() );
	}

	for( size_t i = 0; i < _simplified_cc_unbuildable.size(); ++i )
	{
		for( auto& p : _simplified_cc_unbuildable[ i ].outer() )
			p.x( -p.x() );

		for( auto& inner : _simplified_cc_unbuildable[ i ].inners() )
			for( auto& p : inner )
				p.x( -p.x() );
	}

	for( size_t i = 0; i < _frontiers.size(); ++i )
	{
		//boost::geometry::set<0, 0>( _frontiers[ i ], -boost::geometry::get<0, 0>( _frontiers[ i ] ) );
		//boost::geometry::set<1, 0>( _frontiers[ i ], -boost::geometry::get<1, 0>( _frontiers[ i ] ) );
		for( auto& point : _frontiers[ i ] )
			boost::geometry::set<0>( point, -boost::geometry::get<0>( point ) );
	}
#endif

	// reverse y-axis for the SVG file
	for( size_t i = 0; i < _simplified_cc_level_1.size(); ++i )
	{
		for( auto& p : _simplified_cc_level_1[ i ].outer() )
			p.y( -p.y() );

		for( auto& inner : _simplified_cc_level_1[ i ].inners() )
			for( auto& p : inner )
				p.y( -p.y() );
	}

	for( size_t i = 0; i < _simplified_cc_level_2.size(); ++i )
	{
		for( auto& p : _simplified_cc_level_2[ i ].outer() )
			p.y( -p.y() );

		for( auto& inner : _simplified_cc_level_2[ i ].inners() )
			for( auto& p : inner )
				p.y( -p.y() );
	}

	for( size_t i = 0; i < _simplified_cc_level_3.size(); ++i )
	{
		for( auto& p : _simplified_cc_level_3[ i ].outer() )
			p.y( -p.y() );

		for( auto& inner : _simplified_cc_level_3[ i ].inners() )
			for( auto& p : inner )
				p.y( -p.y() );
	}

	for( size_t i = 0; i < _simplified_cc_unbuildable.size(); ++i )
	{
		for( auto& p : _simplified_cc_unbuildable[ i ].outer() )
			p.y( -p.y() );

		for( auto& inner : _simplified_cc_unbuildable[ i ].inners() )
			for( auto& p : inner )
				p.y( -p.y() );
	}

	for( size_t i = 0; i < _frontiers.size(); ++i )
	{
		//boost::geometry::set<0, 1>( _frontiers[ i ], -boost::geometry::get<0, 1>( _frontiers[ i ] ) );
		//boost::geometry::set<1, 1>( _frontiers[ i ], -boost::geometry::get<1, 1>( _frontiers[ i ] ) );
		for( auto& point : _frontiers[ i ] )
			boost::geometry::set<1>( point, -boost::geometry::get<1>( point ) );
	}

	std::string polygon_mapfile_svg = mapfile;
	polygon_mapfile_svg.replace( polygon_mapfile_svg.end() - 4, polygon_mapfile_svg.end(), "_taunted.svg" );

	std::ofstream polygon_svg( polygon_mapfile_svg );
#if defined SC2API
	boost::geometry::svg_mapper<point> mapper( polygon_svg, 4 * _map_width, 4 * _map_height );
#else
	boost::geometry::svg_mapper<point> mapper( polygon_svg, 5 * _map_width, 5 * _map_height );
#endif

	for( size_t i = 0; i < _simplified_cc_level_1.size(); ++i )
		mapper.add( _simplified_cc_level_1[ i ] );

	for( size_t i = 0; i < _simplified_cc_level_2.size(); ++i )
		mapper.add( _simplified_cc_level_2[ i ] );

	for( size_t i = 0; i < _simplified_cc_level_3.size(); ++i )
		mapper.add( _simplified_cc_level_3[ i ] );

	for( size_t i = 0; i < _simplified_cc_unbuildable.size(); ++i )
		mapper.add( _simplified_cc_unbuildable[ i ] );

	for( size_t i = 0; i < _frontiers.size(); ++i )
		mapper.add( _frontiers[ i ] );

	for( size_t i = 0; i < _simplified_cc_level_1.size(); ++i )
		mapper.map( _simplified_cc_level_1[ i ], "fill-opacity:0.5;fill:rgb(255,255,0);stroke:rgb(0,0,0);stroke-width:5" );

	for( size_t i = 0; i < _simplified_cc_level_2.size(); ++i )
		mapper.map( _simplified_cc_level_2[ i ], "fill-opacity:0.5;fill:rgb(180,255,0);stroke:rgb(0,0,0);stroke-width:5" );

	for( size_t i = 0; i < _simplified_cc_level_3.size(); ++i )
		mapper.map( _simplified_cc_level_3[ i ], "fill-opacity:0.5;fill:rgb(0,255,0);stroke:rgb(0,0,0);stroke-width:5" );

	for( size_t i = 0; i < _simplified_cc_unbuildable.size(); ++i )
		mapper.map( _simplified_cc_unbuildable[ i ], "fill-opacity:0.5;fill:rgb(0,0,255);stroke:rgb(0,0,0);stroke-width:5" );

	for( size_t i = 0; i < _frontiers.size(); ++i )
		mapper.map( _frontiers[ i ], "fill-opacity:1;fill:rgb(255,0,0);stroke:rgb(255,0,0);stroke-width:10" );
}

/***************/
/***   SC2   ***/
/***************/
#ifdef SC2API
terrain_analysis::terrain_analysis( sc2::Agent* bot, analyze_type at )
	: _bot( bot ),
	_analyze_type( at ),
	_last_label( 0 ),
	_small_distance_strategy( 0.01 ),
	_distance_strategy( 0.5 )
{}

int terrain_analysis::compute_terrain_height( int tile_x, int tile_y ) const
{
	auto& grid = _bot->Observation()->GetGameInfo().terrain_height;

	if( tile_x < 0 || tile_x >= grid.width || tile_y < 0 || tile_y >= grid.height )
		return 0;

	unsigned char value = grid.data[ tile_x + tile_y * grid.width ];
	auto temp = std::max( 0.0f, ( static_cast<float>( value ) - 127.0f ) / 16.f - 2 );
	int height = static_cast<int>( std::round( temp ) );
	if( height == 2 )
		height = 0;
	else
		if( height == 3 )
			height = 2;

	return height;
}

bool terrain_analysis::compute_walkable( int tile_x, int tile_y )
{
	return get_bit( _bot->Observation()->GetGameInfo().pathing_grid, tile_x, tile_y );
}

bool terrain_analysis::compute_buildable( int tile_x, int tile_y )
{
	return get_bit( _bot->Observation()->GetGameInfo().placement_grid, tile_x, tile_y );
}

bool terrain_analysis::is_mineral( const unit_type& unit_type ) const
{
	switch( unit_type.ToType() )
	{
	case sc2::UNIT_TYPEID::NEUTRAL_BATTLESTATIONMINERALFIELD: return true;
	case sc2::UNIT_TYPEID::NEUTRAL_BATTLESTATIONMINERALFIELD750: return true;
	case sc2::UNIT_TYPEID::NEUTRAL_LABMINERALFIELD: return true;
	case sc2::UNIT_TYPEID::NEUTRAL_LABMINERALFIELD750: return true;
	case sc2::UNIT_TYPEID::NEUTRAL_MINERALFIELD: return true;
	case sc2::UNIT_TYPEID::NEUTRAL_MINERALFIELD450: return true;
	case sc2::UNIT_TYPEID::NEUTRAL_MINERALFIELD750: return true;
	case sc2::UNIT_TYPEID::NEUTRAL_PURIFIERMINERALFIELD: return true;
	case sc2::UNIT_TYPEID::NEUTRAL_PURIFIERMINERALFIELD750: return true;
	case sc2::UNIT_TYPEID::NEUTRAL_PURIFIERRICHMINERALFIELD: return true;
	case sc2::UNIT_TYPEID::NEUTRAL_PURIFIERRICHMINERALFIELD750: return true;
	case sc2::UNIT_TYPEID::NEUTRAL_RICHMINERALFIELD: return true;
	case sc2::UNIT_TYPEID::NEUTRAL_RICHMINERALFIELD750: return true;
	default: return false;
	}
}

bool terrain_analysis::is_geyser( const unit_type& unit_type ) const
{
	switch( unit_type.ToType() )
	{
	case sc2::UNIT_TYPEID::NEUTRAL_PROTOSSVESPENEGEYSER: return true;
	case sc2::UNIT_TYPEID::NEUTRAL_PURIFIERVESPENEGEYSER: return true;
	case sc2::UNIT_TYPEID::NEUTRAL_RICHVESPENEGEYSER: return true;
	case sc2::UNIT_TYPEID::NEUTRAL_SHAKURASVESPENEGEYSER: return true;
	case sc2::UNIT_TYPEID::NEUTRAL_SPACEPLATFORMGEYSER: return true;
	case sc2::UNIT_TYPEID::NEUTRAL_VESPENEGEYSER: return true;
	default: return false;
	}
}

bool terrain_analysis::get_bit( const sc2::ImageData& grid, int tile_x, int tile_y ) const
{
	// assert( grid.bits_per_pixel == 1 );
	if( tile_x < 0 || tile_x >= grid.width || tile_y < 0 || tile_y >= grid.height )
		return false;

	div_t idx = std::div( tile_x + tile_y * grid.width, 8 );
	return ( grid.data[ idx.quot ] >> ( 7 - idx.rem ) ) & 1;
}

region terrain_analysis::get_region_at( const position& p ) const
{
	return get_region_at( static_cast<int>( p.x ), static_cast<int>( p.y ) );
}

bool terrain_analysis::is_walkable_terrain( const position& p ) const
{
	return is_walkable_terrain( static_cast<int>( p.x ), static_cast<int>( p.y ) );
}

bool terrain_analysis::is_buildable( const position& p ) const
{
	return is_buildable( static_cast<int>( p.x ), static_cast<int>( p.y ) );
}

bool terrain_analysis::is_depot_buildable( const position& p ) const
{
	return is_depot_buildable( static_cast<int>( p.x ), static_cast<int>( p.y ) );
}

int terrain_analysis::get_ground_height( const position& p ) const
{
	return get_ground_height( static_cast<int>( p.x ), static_cast<int>( p.y ) );
}

std::string terrain_analysis::map_filename() const
{
	return _bot->Observation()->GetGameInfo().local_map_path;
}

// TODO: map_filename - map_name
std::string terrain_analysis::map_pathname() const
{
	return _bot->Observation()->GetGameInfo().local_map_path;
}

std::string terrain_analysis::map_name() const
{
	return _bot->Observation()->GetGameInfo().map_name;
}

#else // BWAPI
/**************/
/***   BW   ***/
/**************/
terrain_analysis::terrain_analysis( analyze_type at )
	: _analyze_type( at ),
	_map_width( BWAPI::Broodwar->mapWidth() ),
	_map_height( BWAPI::Broodwar->mapHeight() ),
	_last_label( 0 ),
	_walkable( matrix_bool( _map_height, std::vector<bool>( _map_width, true ) ) ),
	_buildable( matrix_bool( _map_height, std::vector<bool>( _map_width, false ) ) ),
	_depot_buildable( matrix_bool( _map_height, std::vector<bool>( _map_width, false ) ) ),
	_resources( matrix_bool( _map_height, std::vector<bool>( _map_width, false ) ) ),
	_region_id( matrix_int( _map_height, std::vector<int>( _map_width, 0 ) ) ),
	_terrain_height( matrix_int( _map_height, std::vector<int>( _map_width, 0 ) ) ),
	_terrain_properties_level_1( matrix_bool( _map_height, std::vector<bool>( _map_width, false ) ) ),
	_terrain_properties_level_2( matrix_bool( _map_height, std::vector<bool>( _map_width, false ) ) ),
	_terrain_properties_level_3( matrix_bool( _map_height, std::vector<bool>( _map_width, false ) ) ),
	_terrain_unbuildable_unwalkable( matrix_int( _map_height, std::vector<int>( _map_width, 0 ) ) ),
	_small_distance_strategy( 0.01 ),
	_distance_strategy( 0.5 )
{}

int terrain_analysis::compute_terrain_height( int tile_x, int tile_y ) const
{
	return BWAPI::Broodwar->getGroundHeight( tile_x, tile_y );
}

bool terrain_analysis::compute_walkable( int tile_x, int tile_y )
{
	for( int mini_x = 0; mini_x < 4; ++mini_x )
		for( int mini_y = 0; mini_y < 4; ++mini_y )
			if( !BWAPI::Broodwar->isWalkable( tile_x * 4 + mini_x, tile_y * 4 + mini_y ) )
				return false;

	return true;
}

bool terrain_analysis::compute_buildable( int tile_x, int tile_y )
{
	return BWAPI::Broodwar->isBuildable( BWAPI::TilePosition( tile_x, tile_y ) );
}

bool terrain_analysis::is_mineral( const unit_type& unit_type ) const
{
	return unit_type.isMineralField();
}

bool terrain_analysis::is_geyser( const unit_type& unit_type ) const
{
	return unit_type == BWAPI::UnitTypes::Resource_Vespene_Geyser;
}

region terrain_analysis::get_region_at( const position& p ) const
{
	return get_region_at( p.x / 32, p.y / 32 );
}

bool terrain_analysis::is_walkable_terrain( const walk_position& wp ) const
{
	return is_walkable_terrain( wp.x / 4, wp.y / 4 );
}

bool terrain_analysis::is_walkable_terrain( const position& p ) const
{
	return is_walkable_terrain( p.x / 32, p.y / 32 );
}

bool terrain_analysis::is_buildable( const position& p ) const
{
	return is_buildable( p.x / 32, p.y / 32 );
}

bool terrain_analysis::is_depot_buildable( const position& p ) const
{
	return is_depot_buildable( p.x / 32, p.y / 32 );
}

int terrain_analysis::get_ground_height( const position& p ) const
{
	return get_ground_height( p.x / 32, p.y / 32 );
}

std::string terrain_analysis::map_filename() const
{
	return BWAPI::Broodwar->mapFileName();
}

std::string terrain_analysis::map_pathname() const
{
	return BWAPI::Broodwar->mapPathName();
}

std::string terrain_analysis::map_name() const
{
	return BWAPI::Broodwar->mapName();
}
#endif

