#pragma once

#ifdef SC2API
#include "sc2api/sc2_common.h"
#else
#include "BWAPI.h"
#endif

namespace taunt
{
#ifdef SC2API
  using position = sc2::Point2D;
  using tile_position = sc2::Point2DI;
	using walk_position = sc2::Point2D;
  using position_type = float;
#else
  using position = BWAPI::Position;
  using tile_position = BWAPI::TilePosition;
  using walk_position = BWAPI::WalkPosition;
  using position_type = int;
#endif
}
