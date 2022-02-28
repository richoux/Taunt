#include "base_location.hpp"

using namespace taunt;

base_location::base_location( const tile_position& position,
															const std::vector<tile_position>& mineral_positions,
															const std::vector<tile_position>& gas_positions )
	: _position( position ),
	_mineral_positions( mineral_positions ),
	_gas_positions( gas_positions )
{}
