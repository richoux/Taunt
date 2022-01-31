#pragma once

#include <vector>
#include <string>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/linestring.hpp>

#include "analyze_type.hpp"
#include "region.hpp"
#include "positions.hpp"
#include "chokepoint.hpp"

#ifdef SC2API
#include "sc2api/sc2_api.h"
#else
#include "BWAPI.h"
#endif

using matrix_bool = std::vector< std::vector<bool> >;
using matrix_int = std::vector< std::vector<int> >;

using point = boost::geometry::model::d2::point_xy<int>;
using segment = boost::geometry::model::segment<point>;
using boost_polygon = boost::geometry::model::polygon<point>;
using line = boost::geometry::model::linestring<point>;
using multipoint = boost::geometry::model::multi_point<point>;

#ifdef SC2API
using unit_type = sc2::UnitTypeID;
#else
using unit_type = BWAPI::UnitType;
#endif

namespace taunt
{
	class terrain_analysis
	{
#ifdef SC2API
		sc2::Agent* _bot;
#endif
		analyze_type _analyze_type;
		int _map_width;
		int _map_height;

		std::vector< tile_position > _base_locations;
		std::vector< tile_position > _start_base_locations;
		tile_position _start_location;
		std::vector< region > _regions;
		std::vector< chokepoint > _chokepoints;

		// From the CommandCenter bot
		matrix_bool	_walkable;			// whether a tile is buildable (includes static resources)
		matrix_bool	_buildable;			// whether a tile is buildable (includes static resources)
		matrix_bool	_depot_buildable;	// whether a depot is buildable on a tile (illegal within 3 tiles of static resource)
		matrix_bool	_resources;			// whether a tile contains a static resource
		matrix_int	_region_id;			// connectivity sector number, two tiles are ground connected if they have the same number
		matrix_int	_terrain_height;	// height of the map (modification of SC2 heights to match BW heights)

		matrix_bool _terrain_properties_low;
		matrix_bool _terrain_properties_high;
		matrix_bool _terrain_properties_very_high;
		matrix_int  _terrain_unbuildable_unwalkable;

		std::vector<boost_polygon> _simplified_cc_low;
		std::vector<boost_polygon> _simplified_cc_high;
		std::vector<boost_polygon> _simplified_cc_very_high;
		std::vector<boost_polygon> _simplified_cc_unbuildable;
		std::vector<segment> _frontiers;

		void make_frontiers( const std::vector< int >& solution, const std::vector<line>& separations );
		boost_polygon enrich( const boost_polygon& input ) const;

		int compute_terrain_height( int tile_x, int tile_y ) const;
		bool compute_walkable( int tile_x, int tile_y );
		bool compute_buildable( int tile_x, int tile_y );
		bool is_mineral( const unit_type& unit_type ) const;
		bool is_geyser( const unit_type& unit_type ) const;
		inline bool is_valid_tile( const tile_position& tp ) const { return is_valid_tile( tp.x, tp.y ); }
		inline bool is_valid_tile( int tile_x, int tile_y ) const { return tile_x >= 0 && tile_y >= 0 && tile_x < _map_width&& tile_y < _map_height; }
#ifdef SC2API
		bool get_bit( const sc2::ImageData& grid, int tile_x, int tile_y ) const;
#endif

	public:
#ifdef SC2API
		terrain_analysis( sc2::Agent* bot, analyze_type at = analyze_type::SHORTEST_CHOKES );
#else
		terrain_analysis( analyze_type at = analyze_type::SHORTEST_CHOKES );
#endif

		void analyze();
		void print();
		inline void change_analyze_type( analyze_type at ) { _analyze_type = at; }
		inline analyze_type get_analyze_type() const { return _analyze_type; }

		inline std::vector< tile_position > get_base_locations() const { return _base_locations; }
		inline std::vector< tile_position > get_start_base_locations() const { return _start_base_locations; }
		inline tile_position get_start_location() const { return _start_location; }
		tile_position get_nearest_base_location( const tile_position& tp ) const;
		tile_position get_nearest_base_location( const position& p ) const;
		tile_position get_nearest_base_location( int tile_x, int tile_y ) const;

		inline std::vector< region > get_regions() const { return _regions; }
		inline region get_region_at( const tile_position& tp ) const { return get_region_at( tp.x, tp.y ); }
		region get_region_at( const position& p ) const;
		inline region get_region_at( int tile_x, int tile_y ) const { return _regions[ _region_id[ tile_y ][ tile_x ] ]; }

		inline std::vector< chokepoint > get_chokepoints() const { return _chokepoints; }
		// inline chokepoint get_nearest_chokepoint( const tile_position& tp ) const { return get_nearest_point( tp.x, tp.y ); }
		// chokepoint get_nearest_chokepoint( const position& p ) const;
		// chokepoint get_nearest_chokepoint( int tile_x, int tile_y ) const;

		// bool is_connected( const region& r1, const region& r2 ) const;
		// inline bool is_connected( const tile_position& tp1, const tile_position& tp2 ) const { return is_connected( tp1.x, tp1.y, tp2.x, tp2.y ); }
		// bool is_connected( const position& p1, const position& p2 ) const;
		// bool is_connected( int tile_x1, int tile_y1, int tile_x2, int tile_y2 ) const;

		// inline double get_ground_distance( const tile_position& tp1, const tile_position& tp2 ) const { return get_ground_height( tp1.x, tp1.y, tp2.x, tp2.y ); }
		// double get_ground_distance( const position& p1, const position& p2 ) const;
		// double get_ground_distance( int tile_x1, int tile_y1, int tile_x2, int tile_y2 ) const;

		// inline std::vector< tile_position > get_shortest_path( const tile_position& tp ) const { return get_shortest_path( tp.x, tp.y ); }
		// std::vector< tile_position > get_shortest_path( const position& p ) const;
		// std::vector< tile_position > get_shortest_path( int tile_x, int tile_y ) const;

		inline bool is_walkable_terrain( const tile_position& tp ) const { return is_walkable_terrain( tp.x, tp.y ); }
#if not defined SC2API
		bool is_walkable_terrain( const walk_position& wp ) const;
#endif
		bool is_walkable_terrain( const position& p ) const;
		inline bool is_walkable_terrain( int tile_x, int tile_y ) const { return _walkable[ tile_y ][ tile_x ]; }

		inline bool is_buildable( const tile_position& tp ) const { return is_buildable( tp.x, tp.y ); }
		bool is_buildable( const position& p ) const;
		inline bool is_buildable( int tile_x, int tile_y ) const { return _buildable[ tile_y ][ tile_x ]; }

		inline bool is_depot_buildable( const tile_position& tp ) const { return is_depot_buildable( tp.x, tp.y ); }
		bool is_depot_buildable( const position& p ) const;
		inline bool is_depot_buildable( int tile_x, int tile_y ) const { return _depot_buildable[ tile_y ][ tile_x ]; }

		inline int get_ground_height( const tile_position& tp ) const { return get_ground_height( tp.x, tp.y ); }
		int get_ground_height( const position& p ) const;
		inline int get_ground_height( int tile_x, int tile_y ) const { return _terrain_height[ tile_y ][ tile_x ]; }

		std::string map_filename() const;
		std::string map_pathname() const;
		std::string map_name() const;
		inline int map_width() const { return _map_width; }
		inline int map_height() const { return _map_height; }
	};
}
