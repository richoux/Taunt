#pragma once

#include <vector>

namespace taunt
{
  class connected_component
  {
	std::vector< std::vector<int> > _map; // -2 = walkable but unbuidable (slopes, bridges), -1 = unwalkable, 0 = unset, v>0 = label
	size_t _width;
	size_t _height;
	std::vector<int> _labels;
	int _last_label;

	bool is_buildable( size_t x, size_t y );
	void scan_block( size_t x, size_t y );
	void resolve( int label1, int label2 );

  public:
	connected_component( const std::vector< std::vector<int> >& map ); // input map is supposed to be correctly formated with -2, -1 and 0 values only.
	connected_component( std::vector< std::vector<int> >&& map );

	std::vector< std::vector<int> > compute_cc();
  };
}