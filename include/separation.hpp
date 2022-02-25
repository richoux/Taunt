#pragma once
#include <vector>

#include "positions.hpp"
#include "geometry.hpp"

namespace taunt
{
  class region;
  class separation
  {
    std::vector< region* > _region;
    boost_polygon polygon;
    std::vector< position > _contour;
    position _centroid;
  };
}
