#pragma once
#include <vector>

#include "positions.hpp"

namespace taunt
{
  class region;
  class base_location
  {
	  tile_position _position;
	  std::vector< tile_position > _mineral_positions;
	  std::vector< tile_position > _gas_positions;
	  const region* _region;

  public:
		base_location( const tile_position& position, const std::vector<tile_position>& mineral_positions, const std::vector<tile_position>& gas_positions );

		inline tile_position get_position() const { return _position; }
		inline std::vector< tile_position > get_mineral_positions() const {	return _mineral_positions; }
		inline std::vector< tile_position > get_gas_positions() const { return _gas_positions; }
		inline const region* get_region() const { return _region; }
  };
}
