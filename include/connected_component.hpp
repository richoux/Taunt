#pragma once

#include <vector>
#include <utility>
#include <cstddef>

#include "geometry.hpp"

using matrix_bool = std::vector< std::vector<bool> >;
using matrix_int = std::vector< std::vector<int> >;

namespace taunt
{
	enum direction {E, SE, S, SW, W, NW, N, NE};

	class connected_component
	{
		matrix_int _map; // 0 = unset, 1 = unwalkable, 2 = walkable but unbuidable (slopes, bridges), 3 = polygons of unwalkable tiles
		size_t _width;
		size_t _height;
		std::vector<ring> _rings;
		std::vector<bool> _is_inner;
		//matrix_int* _region_id;
		//int _last_label;
		
		bool is_on_map( size_t x, size_t y ) const;
		bool is_walkable( size_t x, size_t y ) const;
		bool has_walkable_around( size_t x, size_t y ) const;
		bool are_all_walkable_around( size_t x, size_t y ) const;

		point previous_point( const point& p, const point& current_point );
		ring neighbors_with_direction( int direction, const point& current_point );
		point look_around( const point& current_point, const point& parent, direction direction );
		void search_for_polygon( int x, int y, direction direction );

		inline bool is_marked( int x, int y ) const { return is_on_map( x, y ) && _map[y][x] == 3; }
		//inline bool has_region_id( int x, int y ) const { return is_on_map( x, y ) && (*_region_id)[y][x] != 0; }
		inline bool is_same_point( const point& point1, const point& point2 ) { return point1.x() == point2.x() && point1.y() == point2.y(); }

		void compute_polygons();
		
	public:
		connected_component( const matrix_int& map ); // input map is supposed to be correctly formatted with 0, 1 and 2 values only.
		connected_component( matrix_int&& map );
		//connected_component( const matrix_bool& map_bool, matrix_int* region_id, int last_label = 0 );
		connected_component( const matrix_bool& map_bool );

		std::vector< boost_polygon > compute_simplified_polygons();
		inline matrix_int get_map() { return _map; }
		inline std::vector<bool> get_inners() { return _is_inner; }
		//inline int get_last_label() const { return _last_label; }
	};
}
