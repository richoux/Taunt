#pragma once

#include <vector>
#include <utility>
#include <cstddef>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/linestring.hpp>

using point = boost::geometry::model::d2::point_xy<int>;
using contour = boost::geometry::model::ring<point>;

namespace taunt
{
	class connected_component
	{
		std::vector< std::vector<int> > _map; // -2 = walkable but unbuidable (slopes, bridges), -1 = unwalkable, 0 = unset, v>0 = label
		size_t _width;
		size_t _height;
		std::vector<int> _labels;
		unsigned int _last_label;
		unsigned int _nb_labels;
		std::vector< contour > _contours;

		bool is_buildable( size_t x, size_t y ) const;
		bool has_buildable_around( size_t x, size_t y ) const;
		bool are_all_buildable_around( size_t x, size_t y ) const;
		void scan_block( size_t x, size_t y );
		void resolve( int label1, int label2 );
		void soft_resolve( int label1, int label2 );
		contour neighbors_with_direction( int direction, const point& current_point );
		point look_around( const point& current_point, const point& parent );
		bool is_same_point( const point& point1, const point& point2 );
		contour search_for_contour( int x, int y );

	public:
		connected_component( const std::vector< std::vector<int> >& map ); // input map is supposed to be correctly formated with -2, -1 and 0 values only.
		connected_component( std::vector< std::vector<int> >&& map );

		std::vector< std::vector<int> > compute_cc();
		std::vector< contour > compute_contours();
		std::vector< contour > compute_simplified_contours();

		enum directions {NE, E, SE, S, SW, W, NW, N};
	};
}
