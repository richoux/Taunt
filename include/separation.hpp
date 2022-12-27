#pragma once
#include <vector>

#include "positions.hpp"
#include "geometry.hpp"

namespace taunt
{

  class separation
  {
    friend class region;
    friend class terrain_analysis;

    int _id;
   	std::vector< const region* > _regions;
    boost_polygon _polygon;
    std::vector< position > _contour;
    position _centroid;

  public:
    separation( int id, const boost_polygon& polygon );

    inline int get_id() const { return _id;  }
    inline std::vector< const region* > get_regions() const { return _regions; }
    inline std::vector< position > get_contour() const { return _contour; }
    inline position get_centroid() const { return _centroid; }
  };
}
