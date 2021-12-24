#pragma once
#include <vector>

#include "base_location.hpp"
#include "chokepoint.hpp"
#include "positions.hpp"
#include "polygon.hpp"

namespace taunt
{
  class region
  {
	  std::vector< base_location > _base_locations;
	  std::vector< chokepoint > _chokepoints;
	  polygon _polygon;
  };
}
