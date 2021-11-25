#pragma once

#include <vector>
#include <utility>
#include <cstddef>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/linestring.hpp>

using point = boost::geometry::model::d2::point_xy<int>;
using contour = boost::geometry::model::polygon<point>;

namespace taunt
{
	class connected_component
	{
		std::vector< std::vector<int> > _map; // -2 = walkable but unbuidable (slopes, bridges), -1 = unwalkable, 0 = unset, v>0 = label
		size_t _width;
		size_t _height;
		unsigned int _last_label;
		std::vector< contour > _contours;

		bool is_on_map( size_t x, size_t y ) const;
		bool is_walkable( size_t x, size_t y ) const;
		// bool has_walkable_around( size_t x, size_t y ) const;
		bool are_all_walkable_around( size_t x, size_t y ) const;
		// bool is_buildable( size_t x, size_t y ) const;
		// bool has_buildable_around( size_t x, size_t y ) const;
		// bool are_all_buildable_around( size_t x, size_t y ) const;

		boost::geometry::model::ring<point> neighbors_with_direction( int direction, const point& current_point );
		point look_around( const point& current_point, const point& parent, bool is_external, int number_inner_rings, int label );
		void search_for_contour( int x, int y, contour& contour, int label );
		void start_external_contour( int x, int y );
		void start_internal_contour( int x, int y, int label );

		inline bool is_labeled( int x, int y ) const { return is_on_map( x, y ) && _map[y][x] > 0; }
		inline bool is_same_point( const point& point1, const point& point2 ) { return point1.x() == point2.x() && point1.y() == point2.y(); }

		// std::vector< std::vector<int> > compute_cc_he(); // fast connected component from He et al.
		std::vector< std::vector<int> > compute_cc_chang(); // contour tracing-based connected component from Chang et al.
		
	public:
		connected_component( const std::vector< std::vector<int> >& map ); // input map is supposed to be correctly formated with -2, -1 and 0 values only.
		connected_component( std::vector< std::vector<int> >&& map );

		inline std::vector< std::vector<int> > compute_cc() { return compute_cc_chang(); }
		inline std::vector< contour > compute_contours() { return _contours; }
		std::vector< contour > compute_simplified_contours();

		enum direction {E, SE, S, SW, W, NW, N, NE};
	};
}
