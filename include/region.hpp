#pragma once
#include <vector>

#include "base_location.hpp"
#include "separation.hpp"
#include "positions.hpp"
#include "geometry.hpp"

namespace taunt
{
  class region
  {
    int _id;
	  std::vector< const base_location* > _base_locations;
	  std::vector< const separation* > _separations;
	  boost_polygon _polygon;
    std::vector< position > _contour;
    position _centroid;

    friend class terrain_analysis;

  public:
    region( int id, const std::vector<const base_location*>& base_locations, const std::vector<const separation*>& separations, const boost_polygon& polygon );

    inline int get_id() const { return _id; }
    inline std::vector< const base_location* > get_base_locations() const { return _base_locations; }
    inline const base_location* get_first_base_locations() const { return _base_locations[ 0 ]; }
    inline std::vector< const separation* > get_separations() const { return _separations; }
    inline std::vector< position > get_contour() const { return _contour; }
    inline position get_centroid() const { return _centroid; }
    inline double get_area() const { return boost::geometry::area( _polygon ); }
    inline double get_perimeter() const { return boost::geometry::perimeter( _polygon ); }
    inline bool is_island() const { return _separations.empty(); }
    position get_nearest_contour_position( const position& pos );
    position get_entry_position( const separation& sep );
  };
}
