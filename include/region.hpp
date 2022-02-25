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
	  std::vector< base_location > _base_locations;
	  std::vector< separation > _separations;
	  boost_polygon _polygon;
  };
}
