#include "connected_component.hpp"

namespace taunt
{
  connected_component::connected_component( const std::vector< std::vector<int> >& map )
	: _map( map ),
	  _height( _map.size() ), // matrices are implemented as [y][x], but all method interfaces are [x][y]
	  _width( _map[ 0 ].size() ),
	  _last_label( 0 )
  {}

  connected_component::connected_component( std::vector< std::vector<int> >&& map )
	: _map( std::move( map ) ),
	  _height( _map.size() ), // matrices are implemented as [y][x], but all method interfaces are [x][y]
	  _width( _map[ 0 ].size() ),
	  _last_label( 0 )
  {}

  bool connected_component::is_buildable( size_t x, size_t y )
  {
	if( x < 0 || x > _width || y < 0 || y > _height )
	  return false;
	else
	  return _map[ y ][ x ] >= 0;
  }

  void connected_component::scan_block( size_t x, size_t y )
  {
	if( !is_buildable( x, y ) )
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
			_map[ y ][ x ] = ++_last_label;
			if( _labels.capacity() < _last_label )
			  _labels.reserve( _last_label * 2 );
			_labels[ _last_label - 1 ] = _last_label; // labels start from 1
		  }
		}
	  }
	}
  }

  void connected_component::resolve( int label1, int label2 )
  { 
	// in this method, we have label1 < label2
	if( label1 > label2 )
	  std::swap( label1, label2 );

	for( size_t i = 0 ; i < _labels.size() ; ++i )
	  if( _labels[ i ] == label2 )
		_labels[ i ] = label1;
  }

  std::vector< std::vector<int> > connected_component::compute_cc()
  {
	// first scan: labels creation
    for( size_t y = 0; y < _height; ++y )
	  for( size_t x = 0; x < _width; ++x )
		scan_block( x, y );

	// second scan: replacement
	for( size_t y = 0; y < _height; ++y )
	  for( size_t x = 0; x < _width; ++x )
		_map[ y ][ x ] = _labels[ _map[ y ][ x ] ];

	return _map;
  }
}