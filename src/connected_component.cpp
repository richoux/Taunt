#include <algorithm>
#include <iostream>

#include "connected_component.hpp"

namespace taunt
{
  connected_component::connected_component( const std::vector< std::vector<int> >& map )
	: _map( map ),
	_height( _map.size() ), // maps are implemented as [y][x], but all methods interface are [x][y]
	_width( _map[ 0 ].size() ),
	_last_label( 0 ),
	_nb_labels( 1 )
  {
	_labels.push_back( 0 ); // first label is 1; this is to have a correspondance _labels[i] = ith label
  }

  connected_component::connected_component( std::vector< std::vector<int> >&& map )
	: _map( std::move( map ) ),
	_height( _map.size() ), // maps are implemented as [y][x], but all methods interface are [x][y]
	_width( _map[ 0 ].size() ),
	_last_label( 0 ),
	_nb_labels( 1 )
  {
	_labels.push_back( 0 ); // first label is 1; this is to have a correspondance _labels[i] = ith label
  }

  bool connected_component::is_buildable( size_t x, size_t y ) const
  {
	if( x < 0 || x >= _width || y < 0 || y >= _height )
	  return false;
	else
	  return _map[ y ][ x ] >= 0;
  }

  bool connected_component::has_buildable_around( size_t x, size_t y ) const
  {
	return is_buildable( x - 1, y - 1 )
	    || is_buildable( x    , y - 1 )
	    || is_buildable( x + 1, y - 1 )
	    || is_buildable( x - 1, y     )
	    || is_buildable( x + 1, y     )
	    || is_buildable( x - 1, y + 1 )
	    || is_buildable( x    , y + 1 )
	    || is_buildable( x + 1, y + 1 );
  }

  bool connected_component::are_all_buildable_around( size_t x, size_t y ) const
  {
	return is_buildable( x - 1, y - 1 )
	  && is_buildable( x, y - 1 )
	  && is_buildable( x + 1, y - 1 )
	  && is_buildable( x - 1, y )
	  && is_buildable( x + 1, y )
	  && is_buildable( x - 1, y + 1 )
	  && is_buildable( x, y + 1 )
	  && is_buildable( x + 1, y + 1 );
  }

  void connected_component::scan_block( size_t x, size_t y )
  {
	//!are_all_buildable_around( x, y ) allows here to consider isolated unbuildable tiles, so surrounded by buildable tiles, to be considered as part of the zone.
	if( !is_buildable( x, y ) && !are_all_buildable_around( x, y ) )
	  return;

	// Algorithm from He et al. - Fast connected-component labeling
	if( is_buildable( x, y - 1 ) )
	  _map[ y ][ x ] = _map[ y - 1 ][ x ];
	else
	{
	  if( is_buildable( x - 1, y ) )
	  {
		_map[ y ][ x ] = _map[ y ][ x - 1 ];
		if( is_buildable( x + 1, y - 1 ) )
		  resolve( _map[ y ][ x - 1 ], _map[ y - 1 ][ x + 1 ] );
	  }
	  else
	  {
		if( is_buildable( x - 1, y - 1 ) )
		{
		  _map[ y ][ x ] = _map[ y - 1 ][ x - 1 ];
		  if( is_buildable( x + 1, y - 1 ) )
			resolve( _map[ y - 1 ][ x - 1 ], _map[ y - 1 ][ x + 1 ] );
		}
		else
		{
		  if( is_buildable( x + 1, y - 1 ) )
			_map[ y ][ x ] = _map[ y - 1 ][ x + 1 ];
		  else
		  {
			if( has_buildable_around( x, y ) )
			{
			  _map[ y ][ x ] = ++_last_label;
			  _labels.push_back( _last_label );
			}
			else
			  _map[ y ][ x ] = -2; // mark as walkable but unbuildable the buildable tiles with no buildable tiles around it
		  }
		}
	  }
	}
  }

  void connected_component::resolve( int label1, int label2 )
  {
	if( label1 == label2 || _labels[ label1 ] == _labels[ label2 ] )
	  return;

	int to_change1 = _labels[ label1 ];
	int to_change2 = _labels[ label2 ];

	// in this method, we have to_change1 < to_change2
	if( to_change1 > to_change2 )
	  std::swap( to_change1, to_change2 );

	for( size_t i = 0; i < _labels.size(); ++i )
	  if( _labels[ i ] == to_change2 )
		_labels[ i ] = to_change1;

	if( _labels[ to_change1 ] != to_change1 )
	  resolve( _labels[ to_change1 ], to_change1 );
  }

  void connected_component::soft_resolve( int label1, int label2 )
  {
	if( label1 == label2 )
	  return;

	// in this method, we have label1 < label2
	if( label1 > label2 )
	  std::swap( label1, label2 );

	for( size_t i = 0; i < _labels.size(); ++i )
	  if( _labels[ i ] == label2 )
		_labels[ i ] = label1;

	if( _labels[ label1 ] > label1 )
	  soft_resolve( _labels[ label1 ], label1 );
  }

  std::vector<std::pair<int, int>> connected_component::neighbors_with_direction( int direction, const std::pair<int, int>& point )
  {
	int x = point.first;
	int y = point.second;

	switch( direction )
	{
	  case directions::E:
		return { std::make_pair( x + 1, y     ),
				 std::make_pair( x + 1, y + 1 ),
				 std::make_pair( x,     y + 1 ),
				 std::make_pair( x - 1, y + 1 ),
		         std::make_pair( x - 1, y     ),
				 std::make_pair( x - 1, y - 1 ),
				 std::make_pair( x,     y - 1 ),
				 std::make_pair( x + 1, y - 1 ),
				 std::make_pair( x + 1, y     ) };

	  case directions::SE:
		return { std::make_pair( x + 1, y + 1 ),
				 std::make_pair( x,     y + 1 ),
				 std::make_pair( x - 1, y + 1 ),
				 std::make_pair( x - 1, y     ),
				 std::make_pair( x - 1, y - 1 ),
				 std::make_pair( x,     y - 1 ),
				 std::make_pair( x + 1, y - 1 ),
		         std::make_pair( x + 1, y     ), 
		         std::make_pair( x + 1, y + 1 ) };

	  case directions::S:
		return { std::make_pair( x,     y + 1 ),
				 std::make_pair( x - 1, y + 1 ),
				 std::make_pair( x - 1, y     ),
				 std::make_pair( x - 1, y - 1 ),
				 std::make_pair( x,     y - 1 ),
				 std::make_pair( x + 1, y - 1 ),
				 std::make_pair( x + 1, y     ),
				 std::make_pair( x + 1, y + 1 ),
				 std::make_pair( x,     y + 1 ) };

	  case directions::SW:
		return { std::make_pair( x - 1, y + 1 ),
				 std::make_pair( x - 1, y     ),
				 std::make_pair( x - 1, y - 1 ),
				 std::make_pair( x,     y - 1 ),
				 std::make_pair( x + 1, y - 1 ),
				 std::make_pair( x + 1, y     ),
				 std::make_pair( x + 1, y + 1 ),
				 std::make_pair( x,     y + 1 ),
		         std::make_pair( x - 1, y + 1 ) };

	  case directions::W:
		return { std::make_pair( x - 1, y     ),
				 std::make_pair( x - 1, y - 1 ),
				 std::make_pair( x,     y - 1 ),
				 std::make_pair( x + 1, y - 1 ),
				 std::make_pair( x + 1, y     ),
				 std::make_pair( x + 1, y + 1 ),
				 std::make_pair( x,     y + 1 ),
				 std::make_pair( x - 1, y + 1 ),
				 std::make_pair( x - 1, y     ) };

	  case directions::NW:
		return { std::make_pair( x - 1, y - 1 ),
				 std::make_pair( x,     y - 1 ),
				 std::make_pair( x + 1, y - 1 ),
				 std::make_pair( x + 1, y     ),
				 std::make_pair( x + 1, y + 1 ),
				 std::make_pair( x,     y + 1 ),
				 std::make_pair( x - 1, y + 1 ),
				 std::make_pair( x - 1, y     ),
		         std::make_pair( x - 1, y - 1 ) };

	  case directions::N:
		return { std::make_pair( x,     y - 1 ),
				 std::make_pair( x + 1, y - 1 ),
				 std::make_pair( x + 1, y     ),
				 std::make_pair( x + 1, y + 1 ),
				 std::make_pair( x,     y + 1 ),
				 std::make_pair( x - 1, y + 1 ),
				 std::make_pair( x - 1, y     ),
				 std::make_pair( x - 1, y - 1 ),
				 std::make_pair( x,     y - 1 ) };

	  default: // NE
		return { std::make_pair( x + 1, y - 1 ),
				 std::make_pair( x + 1, y     ),
				 std::make_pair( x + 1, y + 1 ),
				 std::make_pair( x,     y + 1 ),
				 std::make_pair( x - 1, y + 1 ),
				 std::make_pair( x - 1, y     ),
				 std::make_pair( x - 1, y - 1 ),
				 std::make_pair( x,     y - 1 ),
		         std::make_pair( x + 1, y - 1 ) };
	}
  }

  std::pair<int, int> connected_component::look_around( const std::pair<int, int>& point, const std::pair<int, int>& parent )
  {
	directions direction;
	int count_around = 0;
	if( is_same_point( point, parent ) )
	  direction = directions::NE; // by default
	else
	{
	  for( const auto& n : neighbors_with_direction( directions::E, point ) )
	  {
		if( is_same_point( n, parent ) )
		  break;
		++count_around;
	  }
	  direction = static_cast<directions>( ( count_around + 2 ) % 8 );
	}

	auto neighbors = neighbors_with_direction( direction, point );
	bool has_reached_outside_zone = false;

	for( std::pair<int, int> p : neighbors )
	{
	  if( !is_buildable( p.first, p.second ) )
	  {
		if( !has_reached_outside_zone )
		  has_reached_outside_zone = true;
	  }
	  else
		if( has_reached_outside_zone )
		  return p;
	}

	// should never happen
	return std::make_pair(-1,-1);
  }

  bool connected_component::is_same_point( std::pair<int, int> point1, std::pair<int, int> point2 )
  {
	return point1.first == point2.first && point1.second == point2.second;
  }

  // contour search from Chang et al. - A linear-time component-labeling algorithm using contour tracing technique
  contour connected_component::search_for_contour( int x, int y )
  {
	contour c;
	std::pair<int, int> current_point( x, y );
	c.push_back( current_point );

	while( true )
	{
	  auto parent = c.size() > 1 ? *(c.rbegin()+1) : *c.rbegin();
	  auto next_point = look_around( current_point, parent );

	  c.push_back( next_point );
	  if( is_same_point( next_point, c[ 0 ] ) ) // if the next point is our starting point, we finished the contour.
		return c;
	  else
		current_point = next_point;
	}
  }

  // 2-scan connected-component search algorithm
  std::vector< std::vector<int> > connected_component::compute_cc()
  {
	// first scan: labels creation
	for( size_t y = 0; y < _height; ++y )
	  for( size_t x = 0; x < _width; ++x )
		scan_block( x, y );

	// rename labels from 1 to n
	for( size_t i = 1; i < _labels.size(); ++i )
	  if( _labels[ i ] == _nb_labels + 1 )
		++_nb_labels;
	  else
		if( _labels[ i ] > static_cast<int>( _nb_labels + 1 ) )
		{
		  ++_nb_labels;
		  soft_resolve( _nb_labels, _labels[ i ] );
		}

	// second scan: replacement
	for( size_t y = 0; y < _height; ++y )
	  for( size_t x = 0; x < _width; ++x )
		if( is_buildable( x, y ) )
		  _map[ y ][ x ] = _labels[ _map[ y ][ x ] ];

	return _map;
  }

  std::vector< contour > connected_component::compute_contours()
  {
	if( _nb_labels == 1 ) // if we haven't called compute_cc yet
	  return _contours; // return empty contours

	int nb_contours = 0;
	_contours = std::vector< contour >( _nb_labels );

	for( size_t y = 0; y < _height; ++y )
	  for( size_t x = 0; x < _width; ++x )
	  {
		if( nb_contours == _nb_labels )
		  return _contours;

		if( _map[ y ][ x ] > 0 && _contours[ _map[ y ][ x ] - 1].empty() ) // _map[ y ][ x ] - 1 since labels start at 1 and not at 0
		{
		  _contours[ _map[ y ][ x ] - 1] = search_for_contour( x, y );
		  ++nb_contours;
		}
	  }

	return _contours;
  }
}