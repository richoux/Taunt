#pragma once
#include <vector>

#include "positions.hpp"
#include "geometry.hpp"

namespace taunt
{
  class region;
  class base_location
  {
	  tile_position _position;
	  std::vector< tile_position > _mineral_positions;
	  std::vector< tile_position > _gas_positions;
	  region* _region;
  };
}
