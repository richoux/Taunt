#include <cmath>
#include <fstream>

#include <ghost/solver.hpp>

#include "terrain_analysis.hpp"
#include "connected_component.hpp"
#include "model/region_builder.hpp"
#include "model/print_frontiers.hpp"

using namespace std::literals::chrono_literals;
//using namespace taunt;
using taunt::terrain_analysis;
using taunt::region;

/******************/
/***   Common   ***/
/******************/
void terrain_analysis::make_frontiers( const std::vector<int>& solution,
                                       const std::vector<line>& separations )
{
	for( size_t i = 0 ; i < solution.size() ; ++i )
		if( solution[i] == 1 )
		{
			segment seg{ separations[i][0], separations[i][1] };
			_frontiers.push_back( seg );
		}
}

boost_polygon terrain_analysis::enrich( const boost_polygon& input ) const
{
	boost_polygon output;
	for( auto& inner : input.inners() )
		output.inners().push_back( inner );
	
	int perimeter = static_cast<int>( input.outer().size() );
	
	for( int i = 0 ; i < perimeter - 1 ; ++i )
	{
		boost::geometry::append( output.outer(), input.outer()[i] );

		int next = i + 1;

		auto& current_point = input.outer()[i];
		auto& next_point = input.outer()[next];
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

void terrain_analysis::analyze()
{
	for( int y = 0 ; y < _map_height ; ++y )
		for( int x = 0 ; x < _map_width ; ++x )
		{
			_buildable[y][x]       = compute_buildable(x, y);
			_depot_buildable[y][x] = compute_buildable(x, y);
			_walkable[y][x]        = _buildable[y][x] || compute_walkable(x, y);
			_terrain_height[y][x]  = compute_terrain_height( x, y );
			switch( _terrain_height[y][x] )
			{
			case 0:
				_terrain_properties_low[y][x] = _walkable[y][x];
				break;
			case 2:
				_terrain_properties_high[y][x] = _walkable[y][x];
				break;
			default: // case 4
				_terrain_properties_very_high[y][x] = _walkable[y][x];
			}

			if( !_buildable[y][x] && _walkable[y][x] )
				_terrain_unbuildable_unwalkable[y][x] = 2;
			else
				_terrain_unbuildable_unwalkable[y][x] = 1;
		}

#if defined SC2API
	for( auto& resource : _bot->Observation()->GetUnits() )
	{
		if( !is_mineral( resource.getType() ) && !is_geyser( resource.getType() ) )
			continue;

		int width = resource.getType().tileWidth();
		int height = resource.getType().tileHeight();
		int tile_x = static_cast<int>( std::floor( resource.getPosition().x ) - ( width / 2 ) );
		int tile_y = static_cast<int>( std::floor( resource.getPosition().y ) - ( height / 2 ) );

		for( int y = tile_y ; y < tile_y + height ; ++y )
			for( int x = tile_x ; x < tile_x + width ; ++x )
			{
				_buildable[y][x] = false;
				_resources[y][x] = true;
				_terrain_unbuildable_unwalkable[y][x] = 1;
					
				switch( _terrain_height[y][x] )
				{
				case 0:
					_terrain_properties_low[y][x] = true;
					break;
				case 2:
					_terrain_properties_high[y][x] = true;
					break;
				default: // case 4
					_terrain_properties_very_high[y][x] = true;
				}

				// depots can't be built within 3 tiles of any resource
				for( int ry = -3 ; ry <= 3 ; ++ry )
					for( int rx = -3 ; rx <= 3 ; ++rx )
					{
						// sc2 doesn't fill out the corners of the mineral 3x3 boxes for some reason
						if( std::abs(rx) + std::abs(ry) == 6 )
							continue;
							
						if( !is_valid_tile( x + rx, y + ry ) )
							continue;

						_depot_buildable[ y + ry ][ x + rx ] = false;
					}
			}
	}
#else
	for( auto& resource : BWAPI::Broodwar->getStaticNeutralUnits() )
	{
		if( !resource->getType().isResourceContainer() )
			continue;

		int tile_x = resource->getTilePosition().x;
		int tile_y = resource->getTilePosition().y;

		for( int y = tile_y ; y < tile_y + resource->getType().tileHeight() ; ++y )
			for( int x = tile_x ; x < tile_x + resource->getType().tileWidth() ; ++x )
			{
				_buildable[y][x] = false;
				if( resource->getInitialResources() >= 40 )
				{
					_resources[y][x] = true;
					_terrain_unbuildable_unwalkable[y][x] = 1;
				}
					
				switch( _terrain_height[y][x] )
				{
				case 0:
					_terrain_properties_low[y][x] = true;
					break;
				case 2:
					_terrain_properties_high[y][x] = true;
					break;
				default: // case 4
					_terrain_properties_very_high[y][x] = true;
				}

				// depots can't be built within 3 tiles of any resource
				for( int ry = -3 ; ry <= 3 ; ++ry )
					for( int rx = -3 ; rx <= 3 ; ++rx )
					{
						if( !is_valid_tile( x + rx, y + ry ) )
							continue;

						_depot_buildable[y + ry][x + rx] = false;
					}
			}
	}
#endif
		
	//region_id
	taunt::connected_component cc_low( _terrain_properties_low );
	_simplified_cc_low = cc_low.compute_simplified_contours();
		
	taunt::connected_component cc_high( _terrain_properties_high );
	_simplified_cc_high = cc_high.compute_simplified_contours();
		
	taunt::connected_component cc_very_high( _terrain_properties_very_high );
	_simplified_cc_very_high = cc_very_high.compute_simplified_contours();

	taunt::connected_component cc_unbuildable( _terrain_unbuildable_unwalkable );
	_simplified_cc_unbuildable = cc_unbuildable.compute_simplified_contours();

	size_t number_zones_l = _simplified_cc_low.size();
	size_t number_zones_lh = number_zones_l + _simplified_cc_high.size();
	size_t number_zones_lhvh = number_zones_lh + _simplified_cc_very_high.size();
	//std::vector< boost::geometry::model::multi_point<point> > resources( number_zones_lhvh );
	std::vector< multipoint > resource_clusters( number_zones_lhvh );
	std::vector< int > number_resource_clusters( number_zones_lhvh, 0 );

	std::vector< point > resource_points;
	for( int y = 0 ; y < _map_height ; ++y )
		for( int x = 0 ; x < _map_width ; ++x )
			if( _resources[y][x] )
				resource_points.emplace_back( x, y );

	for( size_t i = 0 ; i < number_zones_l ; ++i )
		for( auto& point : resource_points )
			if( boost::geometry::covered_by( point, _simplified_cc_low[ i ] ) )
				boost::geometry::append( resource_clusters[ i ], point );
	
	for( size_t i = 0 ; i < _simplified_cc_high.size() ; ++i )
		for( auto& point : resource_points )
			if( boost::geometry::covered_by( point, _simplified_cc_high[ i ] ) )
				boost::geometry::append( resource_clusters[ number_zones_l + i ], point );

	for( size_t i = 0 ; i < _simplified_cc_very_high.size() ; ++i )
		for( auto& point : resource_points )
			if( boost::geometry::covered_by( point, _simplified_cc_very_high[ i ] ) )
				boost::geometry::append( resource_clusters[ number_zones_lh + i ], point );

	std::vector< std::vector< multipoint > > clusters_on_the_map( number_zones_lhvh );

#ifdef SC2API
	int max_distance = 17;
#else
	int max_distance = 12;
#endif

	for( size_t i = 0 ; i < resource_clusters.size() ; ++i )
	{
		std::vector< multipoint > clusters;
		for( auto& resource : resource_clusters[i] )
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
				multipoint new_cluster{{ resource }};
				clusters.push_back( new_cluster );
			}
		}

		for( auto& cluster : clusters )
			if( boost::geometry::num_points( cluster ) >= 7 )
				clusters_on_the_map[i].push_back( cluster );
			else
			{
				multipoint temp;
				boost::geometry::difference( resource_clusters[i], cluster, temp );
				resource_clusters[i] = temp;
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
					multipoint temp;
					boost::geometry::union_( clusters[i], clusters[j], temp );
					clusters[i] = temp;
					clusters.erase( clusters.begin() + j );
				}
			}
		}
		
		number_resource_clusters[cpt] = static_cast<int>( clusters.size() );
		++cpt;
	}

	std::vector<boost_polygon> really_simplified_low( _simplified_cc_low.size() );
	std::vector<boost_polygon> really_simplified_high( _simplified_cc_high.size() );
	std::vector<boost_polygon> really_simplified_very_high( _simplified_cc_very_high.size() );
	
	// Compute regions with GHOST
	for( int i = 0 ; i < static_cast<int>( _simplified_cc_low.size() ) ; ++i )
	{
		if( number_resource_clusters[i] >= 2 )
		{
			// boost_polygon temp_simplified;
			// boost::geometry::simplify( _simplified_cc_low[i], temp_simplified, SIMPLIFY);
			// really_simplified_low[i] = enrich( temp_simplified );
			really_simplified_low[i] = enrich( _simplified_cc_low[i] );
				
			double cost;
			std::vector<int> solution( really_simplified_low[i].outer().size() );
			RegionBuilder builder( number_resource_clusters[i] - 1, really_simplified_low[i], clusters_on_the_map[i] );
		
			ghost::Options options;
			options.parallel_runs = true;
			options.tabu_time_selected = builder.separation_candidates.size() / number_resource_clusters[i];
			options.print = std::make_shared<PrintFrontiers>( builder.separation_candidates );
			options.custom_starting_point = true;
			
			ghost::Solver solver( builder );
			if( solver.solve( cost, solution, 100ms, options ) )
				make_frontiers( solution, builder.separation_candidates );
			// else
			// {
			// 	std::cout << "Fail to find a solution for zone " << i << "\n";
			// 	return EXIT_FAILURE;
			// }
		}
	}

	for( int i = 0 ; i < static_cast<int>( _simplified_cc_high.size() ) ; ++i )
	{
		int index = i + number_zones_l;
		if( number_resource_clusters[index] >= 2 )
		{
			// boost_polygon temp_simplified;
			// boost::geometry::simplify( simplified_2[i], temp_simplified, SIMPLIFY);
			// really_simplified_2[i] = enrich( temp_simplified );
			really_simplified_high[i] = enrich( _simplified_cc_high[i] );
	
			double cost;
			std::vector<int> solution( really_simplified_high[i].outer().size() );
			RegionBuilder builder( number_resource_clusters[index] - 1, really_simplified_high[i], clusters_on_the_map[index] );
		
			ghost::Options options;
			options.parallel_runs = true;
			options.tabu_time_selected = builder.separation_candidates.size() / number_resource_clusters[index];
			options.print = std::make_shared<PrintFrontiers>( builder.separation_candidates );
			options.custom_starting_point = true;
		
			ghost::Solver solver( builder );
			if( solver.solve( cost, solution, 100ms, options ) )
				make_frontiers( solution, builder.separation_candidates );
			// else
			// {
			// 	std::cout << "Fail to find a solution for zone " << index << "\n";
			// 	return EXIT_FAILURE;
			// }
		}
	}

	for( int i = 0 ; i < static_cast<int>( _simplified_cc_very_high.size() ) ; ++i )
	{
		int index = i + number_zones_lh;
		if( number_resource_clusters[index] >= 2 )
		{
			// boost_polygon temp_simplified;
			// boost::geometry::simplify( simplified_4[i], temp_simplified, SIMPLIFY);
			// really_simplified_4[i] = enrich( temp_simplified );
			really_simplified_very_high[i] = enrich( _simplified_cc_very_high[i] );

			double cost;
			std::vector<int> solution( really_simplified_very_high[i].outer().size() );
			RegionBuilder builder( number_resource_clusters[index] - 1, really_simplified_very_high[i], clusters_on_the_map[index] );
		
			ghost::Options options;
			options.parallel_runs = true;
			options.tabu_time_selected = builder.separation_candidates.size() / number_resource_clusters[index];
			options.print = std::make_shared<PrintFrontiers>( builder.separation_candidates );
			options.custom_starting_point = true;
		
			ghost::Solver solver( builder );
			if( solver.solve( cost, solution, 100ms, options ) )
				make_frontiers( solution, builder.separation_candidates );
			// else
			// {
			// 	std::cout << "Fail to find a solution for zone " << index << "\n";
			// 	return EXIT_FAILURE;
			// }
		}
	}
}

void terrain_analysis::print()
{
	std::string mapfile = map_filename();
		
#if defined SC2API
	// reverse x-axis for the SVG file from SC2 maps (?!),
	for( size_t i = 0 ; i < _simplified_cc_low.size() ; ++i )
	{
		for( auto& p : _simplified_cc_low[i].outer() )
			p.x( -p.x() );
			
		for( auto& inner : _simplified_cc_low[i].inners() )
			for( auto& p : inner )
				p.x( -p.x() );
	}

	for( size_t i = 0 ; i < _simplified_cc_high.size() ; ++i )
	{
		for( auto& p : _simplified_cc_high[i].outer() )
			p.x( -p.x() );
			
		for( auto& inner : _simplified_cc_high[i].inners() )
			for( auto& p : inner )
				p.x( -p.x() );
	}

	for( size_t i = 0 ; i < _simplified_cc_very_high.size() ; ++i )
	{
		for( auto& p : _simplified_cc_very_high[i].outer() )
			p.x( -p.x() );
			
		for( auto& inner : _simplified_cc_very_high[i].inners() )
			for( auto& p : inner )
				p.x( -p.x() );
	}

	for( size_t i = 0 ; i < _simplified_cc_unbuildable.size() ; ++i )
	{
		for( auto& p : _simplified_cc_unbuildable[i].outer() )
			p.x( -p.x() );
			
		for( auto& inner : _simplified_cc_unbuildable[i].inners() )
			for( auto& p : inner )
				p.x( -p.x() );
	}

	for( size_t i = 0 ; i < _frontiers.size() ; ++i )
	{
		boost::geometry::set<0, 0>( _frontiers[i], - boost::geometry::get<0, 0>( _frontiers[i] ) );
		boost::geometry::set<1, 0>( _frontiers[i], - boost::geometry::get<1, 0>( _frontiers[i] ) );
	}
#endif
	
	// reverse y-axis for the SVG file
	for( size_t i = 0 ; i < _simplified_cc_low.size() ; ++i )
	{
		for( auto& p : _simplified_cc_low[i].outer() )
			p.y( -p.y() );
		
		for( auto& inner : _simplified_cc_low[i].inners() )
			for( auto& p : inner )
				p.y( -p.y() );	
	}

	for( size_t i = 0 ; i < _simplified_cc_high.size() ; ++i )
	{
		for( auto& p : _simplified_cc_high[i].outer() )
			p.y( -p.y() );
		
		for( auto& inner : _simplified_cc_high[i].inners() )
			for( auto& p : inner )
				p.y( -p.y() );	
	}

	for( size_t i = 0 ; i < _simplified_cc_very_high.size() ; ++i )
	{
		for( auto& p : _simplified_cc_very_high[i].outer() )
			p.y( -p.y() );
		
		for( auto& inner : _simplified_cc_very_high[i].inners() )
			for( auto& p : inner )
				p.y( -p.y() );	
	}

	for( size_t i = 0 ; i < _simplified_cc_unbuildable.size() ; ++i )
	{
		for( auto& p : _simplified_cc_unbuildable[i].outer() )
			p.y( -p.y() );
		
		for( auto& inner : _simplified_cc_unbuildable[i].inners() )
			for( auto& p : inner )
				p.y( -p.y() );	
	}

	for( size_t i = 0 ; i < _frontiers.size(); ++i )
	{
		boost::geometry::set<0, 1>( _frontiers[i], - boost::geometry::get<0, 1>( _frontiers[i] ) );
		boost::geometry::set<1, 1>( _frontiers[i], - boost::geometry::get<1, 1>( _frontiers[i] ) );
	}

	std::string contour_mapfile_svg = mapfile;
	contour_mapfile_svg.replace( contour_mapfile_svg.end() - 4, contour_mapfile_svg.end(), "_taunted.svg" );

	std::ofstream contour_svg( contour_mapfile_svg );
	boost::geometry::svg_mapper<point> mapper( contour_svg, 9*_map_width, 9*_map_height );

	for( size_t i = 0 ; i < _simplified_cc_low.size() ; ++i )
		mapper.add( _simplified_cc_low[i] );

	for( size_t i = 0 ; i < _simplified_cc_high.size() ; ++i )
		mapper.add( _simplified_cc_high[i] );

	for( size_t i = 0 ; i < _simplified_cc_very_high.size() ; ++i )
		mapper.add( _simplified_cc_very_high[i] );

	for( size_t i = 0 ; i < _simplified_cc_unbuildable.size() ; ++i )
		mapper.add( _simplified_cc_unbuildable[i] );

	for( size_t i = 0 ; i < _frontiers.size() ; ++i )
		mapper.add( _frontiers[i] );
	
	for( size_t i = 0 ; i < _simplified_cc_low.size() ; ++i )
		mapper.map( _simplified_cc_low[i], "fill-opacity:0.5;fill:rgb(255,255,0);stroke:rgb(0,0,0);stroke-width:5");
	
	for( size_t i = 0 ; i < _simplified_cc_high.size() ; ++i )
		mapper.map( _simplified_cc_high[i], "fill-opacity:0.5;fill:rgb(180,255,0);stroke:rgb(0,0,0);stroke-width:5");
	
	for( size_t i = 0 ; i < _simplified_cc_very_high.size() ; ++i )
		mapper.map( _simplified_cc_very_high[i], "fill-opacity:0.5;fill:rgb(0,255,0);stroke:rgb(0,0,0);stroke-width:5");
	
	for( size_t i = 0 ; i < _simplified_cc_unbuildable.size() ; ++i )
		mapper.map( _simplified_cc_unbuildable[i], "fill-opacity:0.5;fill:rgb(0,0,255);stroke:rgb(0,0,0);stroke-width:5");

	for( size_t i = 0 ; i < _frontiers.size() ; ++i )
		mapper.map( _frontiers[i], "fill-opacity:1;fill:rgb(255,0,0);stroke:rgb(255,0,0);stroke-width:10");
}
	
/***************/
/***   SC2   ***/
/***************/
#ifdef SC2API
terrain_analysis::terrain_analysis( sc2::Agent *bot, analyze_type at )
	: _bot( bot ),
	  _analyze_type( at ),
	  _map_width( _bot->Observation()->GetGameInfo().width ),
	  _map_height( _bot->Observation()->GetGameInfo().height ),
	  _walkable( matrix_bool( _map_height, std::vector<bool>( _map_width, true ) ) ),
	  _buildable( matrix_bool( _map_height, std::vector<bool>( _map_width, false ) ) ),
	  _depot_buildable( matrix_bool( _map_height, std::vector<bool>( _map_width, false ) ) ),
	  _resources( matrix_bool( _map_height, std::vector<bool>( _map_width, false ) ) ),
	  _region_id( matrix_int( _map_height, std::vector<int>( _map_width, -1 ) ) ),
	  _terrain_height( matrix_int( _map_height, std::vector<int>( _map_width, 0 ) ) ),
	  _terrain_properties_low( matrix_bool( _map_height, std::vector<bool>( _map_width, false ) ) ),
	  _terrain_properties_high( matrix_bool( _map_height, std::vector<bool>( _map_width, false ) ) ),
	  _terrain_properties_very_high( matrix_bool( _map_height, std::vector<bool>( _map_width, false ) ) ),
	  _terrain_unbuildable_unwalkable( matrix_int( _map_height, std::vector<int>( _map_width, 0 ) ) )
{}
	
int terrain_analysis::compute_terrain_height( int tile_x, int tile_y ) const
{
	auto& grid = _bot->Observation()->GetGameInfo().terrain_height;
		
	if( tile_x < 0 || tile_x >= grid.width || tile_y < 0 || tile_y >= grid.height )
		return 0;
		
	unsigned char value = grid.data[ tile_x + tile_y * grid.width ];
	auto temp = std::max( 0.0f, ( static_cast<float>(value) - 127.0f ) / 16.f - 2 );
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
	case sc2::UNIT_TYPEID::NEUTRAL_BATTLESTATIONMINERALFIELD    : return true;
	case sc2::UNIT_TYPEID::NEUTRAL_BATTLESTATIONMINERALFIELD750 : return true;
	case sc2::UNIT_TYPEID::NEUTRAL_LABMINERALFIELD		          : return true;
	case sc2::UNIT_TYPEID::NEUTRAL_LABMINERALFIELD750	          : return true;
	case sc2::UNIT_TYPEID::NEUTRAL_MINERALFIELD                 : return true;
	case sc2::UNIT_TYPEID::NEUTRAL_MINERALFIELD450              : return true;
	case sc2::UNIT_TYPEID::NEUTRAL_MINERALFIELD750              : return true;
	case sc2::UNIT_TYPEID::NEUTRAL_PURIFIERMINERALFIELD		      : return true;
	case sc2::UNIT_TYPEID::NEUTRAL_PURIFIERMINERALFIELD750	    : return true;
	case sc2::UNIT_TYPEID::NEUTRAL_PURIFIERRICHMINERALFIELD		  : return true;
	case sc2::UNIT_TYPEID::NEUTRAL_PURIFIERRICHMINERALFIELD750	: return true;
	case sc2::UNIT_TYPEID::NEUTRAL_RICHMINERALFIELD             : return true;
	case sc2::UNIT_TYPEID::NEUTRAL_RICHMINERALFIELD750          : return true;
	default: return false;
	}
}
	
bool terrain_analysis::is_geyser( const unit_type& unit_type ) const
{
	switch( unit_type.ToType() ) 
	{
	case sc2::UNIT_TYPEID::NEUTRAL_PROTOSSVESPENEGEYSER  : return true;
	case sc2::UNIT_TYPEID::NEUTRAL_PURIFIERVESPENEGEYSER : return true;
	case sc2::UNIT_TYPEID::NEUTRAL_RICHVESPENEGEYSER     : return true;
	case sc2::UNIT_TYPEID::NEUTRAL_SHAKURASVESPENEGEYSER : return true;
	case sc2::UNIT_TYPEID::NEUTRAL_SPACEPLATFORMGEYSER   : return true;
	case sc2::UNIT_TYPEID::NEUTRAL_VESPENEGEYSER         : return true;
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
	return get_region_at( static_cast<int>(p.x), static_cast<int>(p.y) );
}

bool terrain_analysis::is_walkable_terrain( const walk_position& wp ) const
{
	return is_walkable_terrain( static_cast<int>(wp.x), static_cast<int>(wp.y) );
}
	
bool terrain_analysis::is_walkable_terrain( const position& p ) const
{
	return is_walkable_terrain( static_cast<int>(p.x), static_cast<int>(p.y) );
}

bool terrain_analysis::is_buildable( const position& p ) const
{
	return is_buildable( static_cast<int>(p.x), static_cast<int>(p.y) );
}

bool terrain_analysis::is_depot_buildable( const position& p ) const
{
	return is_depot_buildable( static_cast<int>(p.x), static_cast<int>(p.y) );
}

int terrain_analysis::get_ground_height( const position& p ) const
{
	return get_ground_height( static_cast<int>(p.x), static_cast<int>(p.y) );
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
	  _walkable( matrix_bool( _map_height, std::vector<bool>( _map_width, true ) ) ),
	  _buildable( matrix_bool( _map_height, std::vector<bool>( _map_width, false ) ) ),
	  _depot_buildable( matrix_bool( _map_height, std::vector<bool>( _map_width, false ) ) ),
	  _resources( matrix_bool( _map_height, std::vector<bool>( _map_width, false ) ) ),
	  _region_id( matrix_int( _map_height, std::vector<int>( _map_width, -1 ) ) ),
	  _terrain_height( matrix_int( _map_height, std::vector<int>( _map_width, 0 ) ) )
{}

int terrain_analysis::compute_terrain_height( int tile_x, int tile_y ) const
{
	return BWAPI::Broodwar->getGroundHeight( tile_x, tile_y );
}
	
bool terrain_analysis::compute_walkable( int tile_x, int tile_y ) 
{
	for( int mini_x = 0 ; mini_x < 4 ; ++mini_x )
		for( int mini_y = 0 ; mini_y < 4 ; ++mini_y )
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

