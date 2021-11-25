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
		  _contours( std::vector<contour>() )
	{}
	
	connected_component::connected_component( std::vector< std::vector<int> >&& map )
		: _map( std::move( map ) ),
		  _height( _map.size() ), // maps are implemented as [y][x], but all methods interface are [x][y]
		  _width( _map[ 0 ].size() ),
		  _last_label( 0 ),
		  _contours( std::vector<contour>() )
	{}

	bool connected_component::is_on_map( size_t x, size_t y ) const
	{
		return x >= 0 && x < _width && y >= 0 && y < _height;
	}
	
	bool connected_component::is_walkable( size_t x, size_t y ) const
	{
		if( !is_on_map( x, y ) )
			return false;
		else
			return _map[ y ][ x ] == -2 || _map[ y ][ x ] == 0 || _map[ y ][ x ] >= 1000;
	}

	// bool connected_component::has_walkable_around( size_t x, size_t y ) const
	// {
	// 	return is_walkable( x - 1, y - 1 )
	// 		|| is_walkable( x    , y - 1 )
	// 		|| is_walkable( x + 1, y - 1 )
	// 		|| is_walkable( x - 1, y     )
	// 		|| is_walkable( x + 1, y     )
	// 		|| is_walkable( x - 1, y + 1 )
	// 		|| is_walkable( x    , y + 1 )
	// 		|| is_walkable( x + 1, y + 1 );
	// }

	bool connected_component::are_all_walkable_around( size_t x, size_t y ) const
	{
		return ( is_walkable( x - 1, y - 1 )  || !is_on_map( x - 1, y - 1 ) )
			  && ( is_walkable( x    , y - 1 )  || !is_on_map( x    , y - 1 ) )
			  && ( is_walkable( x + 1, y - 1 )  || !is_on_map( x + 1, y - 1 ) )
			  && ( is_walkable( x - 1, y     )  || !is_on_map( x - 1, y     ) )
			  && ( is_walkable( x + 1, y     )  || !is_on_map( x + 1, y     ) )
			  && ( is_walkable( x - 1, y + 1 )  || !is_on_map( x - 1, y + 1 ) )
			  && ( is_walkable( x    , y + 1 )  || !is_on_map( x    , y + 1 ) )
			  && ( is_walkable( x + 1, y + 1 )  || !is_on_map( x + 1, y + 1 ) );
	}

	// bool connected_component::is_buildable( size_t x, size_t y ) const
	// {
	// 	if( !is_on_map( x, y ) )
	// 		return false;
	// 	else
	// 		return _map[ y ][ x ] >= 0 && _map[ y ][ x ] < 1000;
	// }

	// bool connected_component::has_buildable_around( size_t x, size_t y ) const
	// {
	// 	return is_buildable( x - 1, y - 1 )
	// 		|| is_buildable( x    , y - 1 )
	// 		|| is_buildable( x + 1, y - 1 )
	// 		|| is_buildable( x - 1, y     )
	// 		|| is_buildable( x + 1, y     )
	// 		|| is_buildable( x - 1, y + 1 )
	// 		|| is_buildable( x    , y + 1 )
	// 		|| is_buildable( x + 1, y + 1 );
	// }

	// bool connected_component::are_all_buildable_around( size_t x, size_t y ) const
	// {
	// 	return is_buildable( x - 1, y - 1 )
	// 		&& is_buildable( x, y - 1 )
	// 		&& is_buildable( x + 1, y - 1 )
	// 		&& is_buildable( x - 1, y )
	// 		&& is_buildable( x + 1, y )
	// 		&& is_buildable( x - 1, y + 1 )
	// 		&& is_buildable( x, y + 1 )
	// 		&& is_buildable( x + 1, y + 1 );
	// }

	boost::geometry::model::ring<point> connected_component::neighbors_with_direction( int direction, const point& current_point )
	{
		int x = current_point.x();
		int y = current_point.y();

		switch( direction )
		{
		case direction::E:
			return {{ x + 1, y     },
			        { x + 1, y + 1 },
			        { x,     y + 1 },
			        { x - 1, y + 1 },
			        { x - 1, y     },
			        { x - 1, y - 1 },
			        { x,     y - 1 },
			        { x + 1, y - 1 },
			        { x + 1, y     }};

		case direction::SE:
			return {{ x + 1, y + 1 },
			        { x,     y + 1 },
			        { x - 1, y + 1 },
			        { x - 1, y     },
			        { x - 1, y - 1 },
			        { x,     y - 1 },
			        { x + 1, y - 1 },
			        { x + 1, y     },
			        { x + 1, y + 1 }};
			        
		case direction::S:
			return {{ x,     y + 1 },
			        { x - 1, y + 1 },
			        { x - 1, y     },
			        { x - 1, y - 1 },
			        { x,     y - 1 },
			        { x + 1, y - 1 },
			        { x + 1, y     },
			        { x + 1, y + 1 },
			        { x,     y + 1 }};

		case direction::SW:
			return {{ x - 1, y + 1 },
			        { x - 1, y     },
			        { x - 1, y - 1 },
			        { x,     y - 1 },
			        { x + 1, y - 1 },
			        { x + 1, y     },
			        { x + 1, y + 1 },
			        { x,     y + 1 },
			        { x - 1, y + 1 }};

		case direction::W:
			return {{ x - 1, y     },
			        { x - 1, y - 1 },
			        { x,     y - 1 },
			        { x + 1, y - 1 },
			        { x + 1, y     },
			        { x + 1, y + 1 },
			        { x,     y + 1 },
			        { x - 1, y + 1 },
			        { x - 1, y     }};

		case direction::NW:
			return {{ x - 1, y - 1 },
			        { x,     y - 1 },
			        { x + 1, y - 1 },
			        { x + 1, y     },
			        { x + 1, y + 1 },
			        { x,     y + 1 },
			        { x - 1, y + 1 },
			        { x - 1, y     },
			        { x - 1, y - 1 }};

		case direction::N:
			return {{ x,     y - 1 },
			        { x + 1, y - 1 },
			        { x + 1, y     },
			        { x + 1, y + 1 },
			        { x,     y + 1 },
			        { x - 1, y + 1 },
			        { x - 1, y     },
			        { x - 1, y - 1 },
			        { x,     y - 1 }};

		default: // NE
			return {{ x + 1, y - 1 },
			        { x + 1, y     },
			        { x + 1, y + 1 },
			        { x,     y + 1 },
			        { x - 1, y + 1 },
			        { x - 1, y     },
			        { x - 1, y - 1 },
			        { x,     y - 1 },
			        { x + 1, y - 1 }};
		}
	}

	point connected_component::look_around( const point& current_point, const point& parent, bool is_external, int number_inner_rings, int label )
	{
		direction direction;
		int count_around = 0;
		if( is_same_point( current_point, parent ) )
		{
			if( is_external )
				direction = direction::NE; // by default for external contours
			else
				direction = direction::SW; // by default for internal contours
		}
		else
		{
			for( const auto& n : neighbors_with_direction( direction::E, current_point ) )
			{
				if( is_same_point( n, parent ) )
					break;
				++count_around;
			}
			direction = static_cast<connected_component::direction>( ( count_around + 2 ) % 8 );
		}

		auto neighbors = neighbors_with_direction( direction, current_point );

		point next_point(-1, -1);
		bool next_point_found = false;
		for( auto& p : neighbors )
		{
			if( is_walkable( p.x(), p.y() ) || !is_on_map( p.x(), p.y() ) )
			{
				if( is_on_map( p.x(), p.y() ) && ( _map[p.y()][p.x()] == 0 || _map[p.y()][p.x()] == -2 ) )
					if( is_external )
						_map[p.y()][p.x()] = 1000 * label;
					else
						_map[p.y()][p.x()] = 1000 * label + number_inner_rings;
			}
			else
				if( !next_point_found )
				{
					next_point_found = true;
					next_point = p;
				}
		}
		
		// should never return (-1, -1)
		return next_point;
	}

	// contour search from Chang et al. - A linear-time component-labeling algorithm using contour tracing technique
	void connected_component::search_for_contour( int x, int y, contour& contour, int label )
	{
		bool is_external = contour.outer().empty();
		point starting_point( x, y );
		point current_point( x, y );
		int inner_ring_index = contour.inners().size();

		contour::ring_type *ring;
		if( is_external )
			ring = &contour.outer();
		else
		{
			contour.inners().push_back( boost::geometry::model::ring<point>() );
			ring = &contour.inners()[ inner_ring_index ];
		}
		
		boost::geometry::append( *ring, current_point );

		// we can't have next_point == starting_point at the first iteration of the loop
		// since Taunt doesn't consider isolated unwalkable tiles.
		do
		{
			// take the second to last point if contour contains at least 2 points, or the last one otherwise.
			auto parent = ring->size() > 1 ? *(ring->rbegin()+1) : *ring->rbegin();

			// inner_ring_index+1 because we need the current number of inner rings, not the current index.
			auto next_point = look_around( current_point, parent, is_external, inner_ring_index + 1, label ); 

			boost::geometry::append( *ring, next_point );
			if( !is_labeled( next_point.x(), next_point.y() ) )
				_map[ next_point.y() ][ next_point.x() ] = label;
	
			current_point = next_point;
		}	while( !is_same_point( current_point, starting_point ) );
	}

	void connected_component::start_external_contour( int x, int y )
	{
		++_last_label;
		_map[y][x] = _last_label;
		_contours.push_back( contour() );

		// _last_label - 1 because indexes start at 0 and labels start at 1
		search_for_contour( x, y, _contours[ _last_label - 1 ], _last_label );
	}

	void connected_component::start_internal_contour( int x, int y, int label )
	{
		search_for_contour( x, y, _contours[ _last_label - 1 ], label );
	}

	// 2-scan connected-component search algorithm from He et al. - Fast connected-component labeling
	// std::vector< std::vector<int> > connected_component::compute_cc_he()
	// {
	// 	// first scan: labels creation
	// 	for( size_t y = 0; y < _height; ++y )
	// 		for( size_t x = 0; x < _width; ++x )
	// 			scan_block( x, y );

	// 	// rename labels from 1 to n
	// 	for( size_t i = 1; i < _labels.size(); ++i )
	// 		if( _labels[ i ] == _nb_labels + 1 )
	// 			++_nb_labels;
	// 		else
	// 			if( _labels[ i ] > static_cast<int>( _nb_labels + 1 ) )
	// 			{
	// 				++_nb_labels;
	// 				soft_resolve( _nb_labels, _labels[ i ] );
	// 			}

	// 	// second scan: replacement
	// 	for( size_t y = 0; y < _height; ++y )
	// 		for( size_t x = 0; x < _width; ++x )
	// 			if( is_walkable( x, y ) )
	// 				_map[ y ][ x ] = _labels[ _map[ y ][ x ] ];

	// 	return _map;
	// }
	
	// contour search from Chang et al. - A linear-time component-labeling algorithm using contour tracing technique
	std::vector< std::vector<int> > connected_component::compute_cc_chang()
	{
		for( size_t y = 0; y < _height; ++y )
			for( size_t x = 0; x < _width; ++x )
				if( !is_walkable( x, y ) && !are_all_walkable_around( x, y ))
				{
					if( !is_labeled( x, y ) && ( is_walkable( x, y - 1 ) || !is_on_map( x, y - 1 ) ) )
						start_external_contour( x, y );

					if( !is_labeled( x, y + 1 ) && is_walkable( x, y + 1 ) )
					{
						int label = ( is_on_map( x - 1, y ) ? _map[y][x-1] : _map[y-1][x] );
						if( label >= 1000 )
						{
							label -= ( label % 1000 );
							label /= 1000;
						}
						start_internal_contour( x, y, label );
					}
					
					if( !is_labeled( x, y ) )
						if( is_on_map( x - 1, y ) )
							_map[y][x] = _map[y][x-1];
						else
							_map[y][x] = _map[y-1][x];
				}

		return _map;
	}

	std::vector< contour > connected_component::compute_simplified_contours()
	{
		// if compute_cc has not been called, return empty contours.
		if( _last_label == 0 )
			return _contours;
		
		std::vector< contour > simplified_contours( _contours.size() );

		for( size_t i = 0 ; i < _contours.size(); ++i )
		{
			boost::geometry::correct( _contours[i] );
			boost::geometry::simplify( _contours[i].outer(), simplified_contours[i].outer(), 1);
			for( size_t j = 0 ; j < _contours[i].inners().size(); ++j )
			{
				simplified_contours[i].inners().push_back( boost::geometry::model::ring<point>() );
				boost::geometry::simplify( _contours[i].inners()[j], simplified_contours[i].inners()[j], 1);
			}
			boost::geometry::correct( simplified_contours[i] );
		}
		
		return simplified_contours;
	}
}
