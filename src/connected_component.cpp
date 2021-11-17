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
	_contours( std::vector<std::vector<bool>>( _height ) )
  {
	_labels.push_back( 0 ); // first label is 1; this is to have a correspondance _labels[i] = ith label
	for( auto& c : _contours )
	  c = std::vector<bool>( _width );
  }

  connected_component::connected_component( std::vector< std::vector<int> >&& map )
	: _map( std::move( map ) ),
	_height( _map.size() ), // maps are implemented as [y][x], but all methods interface are [x][y]
	_width( _map[ 0 ].size() ),
	_last_label( 0 ),
	_contours( std::vector<std::vector<bool>>( _height ) )
  {
	_labels.push_back( 0 ); // first label is 1; this is to have a correspondance _labels[i] = ith label
	for( auto& c : _contours )
	  c = std::vector<bool>( _width, false );
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


  void connected_component::scan_block( size_t x, size_t y )
  {
	if( !is_buildable( x, y ) )
	{
	  // contouring
	  if( is_buildable( x - 1, y - 1 ) )
		_contours[ y - 1 ][ x - 1 ] = true;
	  if( is_buildable( x, y - 1) )
		_contours[ y - 1 ][ x ] = true;
	  if( is_buildable( x + 1, y - 1 ) )
		_contours[ y - 1 ][ x + 1 ] = true;
	  if( is_buildable( x - 1, y ) )
		_contours[ y ][ x - 1 ] = true;
	  if( is_buildable( x + 1, y ) )
		_contours[ y ][ x + 1 ] = true;
	  if( is_buildable( x - 1, y + 1 ) )
		_contours[ y + 1 ][ x - 1 ] = true;
	  if( is_buildable( x, y + 1 ) )
		_contours[ y + 1 ][ x ] = true;
	  if( is_buildable( x + 1, y + 1 ) )
		_contours[ y + 1 ][ x + 1 ] = true;
	  return;
	}

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
			  //_contours[ y ][ x ] = true;
			}
			else
			{
			  _map[ y ][ x ] = -2; // mark as walkable but unbuildable the buildable tiles with no buildable tiles around it
			}
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

	//std::cout << "Resolving " << to_change2 << " and " << to_change1 << "...\n";

	for( size_t i = 0; i < _labels.size(); ++i )
	  if( _labels[ i ] == to_change2 )
		_labels[ i ] = to_change1;

	if( _labels[ to_change1 ] != to_change1 )
	{
	  //std::cout << "recursive call: must resolve " << _labels[ to_change1 ] << " and " << to_change1 << "\n";
	  resolve( _labels[ to_change1 ], to_change1 );
	}
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

	//std::cout << "Soft resolved " << label2 << " with " << label1 << "\n";
  }

  std::vector< std::vector<int> > connected_component::compute_cc()
  {
	// first scan: labels creation
	for( size_t y = 0; y < _height; ++y )
	  for( size_t x = 0; x < _width; ++x )
		scan_block( x, y );

	int label = 1;
	for( size_t i = 1; i < _labels.size(); ++i )
	  if( _labels[ i ] == label + 1 )
		++label;
	  else
		if( _labels[ i ] > label + 1 )
		{
		  ++label;
		  //std::cout << "label=" << label << ", labels[" << i << "]=" << _labels[ i ] << "\n";
		  soft_resolve( label, _labels[ i ] );
		}

	//for( size_t y = 0; y < _height; ++y )
	//  for( size_t x = 0; x < _width; ++x )
	//	if( is_buildable( x, y ) )
	//	  std::cout << "(x=" << x << ",y=" << y << ") label[" << _map[ y ][ x ] << "] = " << _labels[ _map[ y ][ x ] ] << "\n";

	// second scan: replacement
	for( size_t y = 0; y < _height; ++y )
	  for( size_t x = 0; x < _width; ++x )
		if( is_buildable( x, y ) )
		  _map[ y ][ x ] = _labels[ _map[ y ][ x ] ];

	return _map;
  }
}