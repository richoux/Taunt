#pragma once
#include <vector>

#include "positions.hpp"
#include "geometry.hpp"

namespace taunt
{
  class terrain_analysis;
  class base_location;
  class separation;

  class region
  {
    int _id;
	  std::vector< int > _base_locations_indexes;
	  std::vector< int > _separations_indexes;
    terrain_analysis *_ta;
	  boost_polygon _polygon;
    std::vector< position > _contour;
    position _centroid;

  public:
    region( int id, terrain_analysis *_terrain_analysis, const boost_polygon& polygon );

    inline int get_id() const { return _id; }
    inline std::vector< position > get_contour() const { return _contour; }
    inline position get_centroid() const { return _centroid; }
    inline double get_area() const { return boost::geometry::area( _polygon ); }
    inline double get_perimeter() const { return boost::geometry::perimeter( _polygon ); }
    inline bool is_island() const { return _separations_indexes.empty(); }

    std::vector< base_location > get_base_locations() const;
    const base_location& get_first_base_locations() const;
    std::vector< separation > get_separations() const;
  	position get_nearest_contour_position( const position& pos );
    position get_entry_position( const separation& sep );
  };
}
