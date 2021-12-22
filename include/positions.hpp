#pragma once

namespace taunt
{
#ifdef SC2API
#include "sc2api/sc2_common.h"
  using position = sc2::Point2D;
  using tile_position = sc2::Point2DI;
  using position_type = float;

#else
#include "BWAPI.h"
  using position = BWAPI::Position;
  using tile_position = BWAPI::TilePosition;
  using position_type = int;
#endif
}