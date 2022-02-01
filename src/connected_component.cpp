#include <algorithm>

#include "connected_component.hpp"

namespace taunt
{
	connected_component::connected_component( const std::vector< std::vector<int> >& map )
		: _map( map ),
		  _height( _map.size() ), // maps are implemented as [y][x], but all methods interface are [x][y]
		  _width( _map[ 0 ].size() ),
		  _contours( std::vector<contour>() )
	{}
	
	connected_component::connected_component( std::vector< std::vector<int> >&& map )
		: _map( std::move( map ) ),
		  _height( _map.size() ), // maps are implemented as [y][x], but all methods interface are [x][y]
		  _width( _map[ 0 ].size() ),
		  _contours( std::vector<contour>() )
	{}

	connected_component::connected_component( const std::vector< std::vector<bool> >& map_bool, std::vector< std::vector<int> >& region_id, int last_label )
		: _map( std::vector< std::vector<int> >( map_bool.size(), std::vector<int>( map_bool[0].size(), 0 ) ) ),
		  _height( _map.size() ), // maps are implemented as [y][x], but all methods interface are [x][y]
		  _width( _map[ 0 ].size() ),
		  _contours( std::vector<contour>() ),
		  _region_id( region_id ),
		  _last_label( last_label )
	{
		for( size_t y = 0; y < _height; ++y )
			for( size_t x = 0; x < _width; ++x )
				if( map_bool[ y ][ x ] )
					_map[ y ][ x ] = 0;
				else
					_map[ y ][ x ] = 1;
	}

	bool connected_component::is_on_map( size_t x, size_t y ) const
	{
		return x >= 0 && x < _width && y >= 0 && y < _height;
	}
	
	bool connected_component::is_walkable( size_t x, size_t y ) const
	{
		return is_on_map( x, y ) && _map[ y ][ x ] != 1;
	}

	bool connected_component::has_walkable_around( size_t x, size_t y ) const
	{
		return is_walkable( x - 1, y - 1 )
			|| is_walkable( x    , y - 1 )
			|| is_walkable( x + 1, y - 1 )
			|| is_walkable( x - 1, y     )
			|| is_walkable( x + 1, y     )
			|| is_walkable( x - 1, y + 1 )
			|| is_walkable( x    , y + 1 )
			|| is_walkable( x + 1, y + 1 );
	}

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

	point connected_component::previous_point( const point& p, const point& current_point )
	{
		int diff_x = p.x() - current_point.x();
		int diff_y = p.y() - current_point.y();

		if( diff_x == -1 && diff_y == -1 )
			return point( p.x(), current_point.y() );
		
		if( diff_x == -1 && diff_y == 0 )
			return point( p.x(), p.y() + 1 );
				
		if( diff_x == -1 && diff_y == 1 )
			return point( current_point.x(), p.y() );

		if( diff_x == 0 && diff_y == 1 )
			return point( p.x() + 1, p.y() );

		if( diff_x == 1 && diff_y == 1 )
			return point( p.x(), current_point.y() );

		if( diff_x == 1 && diff_y == 0 )
			return point( p.x(), p.y() - 1 );

		if( diff_x == 1 && diff_y == -1 )
			return point( current_point.x(), p.y() );

		return point( p.x() - 1, p.y() ); // if( diff_x == 0 && diff_y == -1 )
	}
	
	point connected_component::look_around( const point& current_point, const point& parent, direction direction )
	{
		int x = current_point.x();
		int y = current_point.y();
		
		if( !is_same_point( current_point, parent ) )
		{
			int count_around = 0;
			for( const auto& n : neighbors_with_direction( direction::E, current_point ) )
			{
				if( is_same_point( n, parent ) )
					break;
				++count_around;
			}
			direction = static_cast<taunt::direction>( ( count_around + 2 ) % 8 );
		}
		
		auto neighbors = neighbors_with_direction( direction, current_point );

		for( auto& p : neighbors )
		{
			if( is_walkable( p.x(), p.y() ) )
				return p;
		}
		
		// should never happen
		return point( -1, -1 );
	}

	// contour search from Chang et al. - A linear-time component-labeling algorithm using contour tracing technique
	void connected_component::search_for_contour( int x, int y, direction direction )
	{
		_contours.push_back( contour() );
		auto& contour = *_contours.rbegin();
		point starting_point( x, y );
		point current_point( x, y );
		boost::geometry::append( contour, current_point );

		int label;

		if( !has_region_id( x, y ) )
		{
			if( has_region_id( x - 1, y ) )
				label = _region_id[ y ][ x - 1 ];
			else
				if( has_region_id( x, y - 1 ) )
					label = _region_id[ y - 1 ][ x ];
				else
					label = ++_last_label; // new label
		}
		else // should never happen
			label = _region_id[ y ][ x ];

		// we can't have next_point == starting_point at the first iteration of the loop
		// since Taunt doesn't consider isolated unwalkable tiles.
		do
		{
			_map[ current_point.y() ][ current_point.x() ] = 3;
			_region_id[ current_point.y() ][ current_point.x() ] = label;
			// take the second to last point if contour contains at least 2 points, or the last one otherwise.
			auto parent = contour.size() > 1 ? *(contour.rbegin()+1) : *contour.rbegin();
			auto next_point = look_around( current_point, parent, direction ); 
			boost::geometry::append( contour, next_point );
			current_point = next_point;
		}
		while( !is_same_point( current_point, starting_point ) );
	}

	// contour search from Chang et al. - A linear-time component-labeling algorithm using contour tracing technique
	void connected_component::compute_contours()
	{
		for( size_t y = 0; y < _height; ++y )
			for( size_t x = 0; x < _width; ++x )
				if( is_walkable( x, y ) && !is_marked( x, y ) && has_walkable_around( x, y ) )
				{
					if( !is_walkable( x, y + 1 ) )
						search_for_contour( x, y, direction::S );
					else
						if( !is_walkable( x + 1, y ) )
							search_for_contour( x, y, direction::E );
						else
							if( !is_walkable( x, y - 1 ) )
								search_for_contour( x, y, direction::N );
							else
								if( !is_walkable( x - 1, y ) )
									search_for_contour( x, y, direction::W );
								else // walkable tile inside a walkable area
									if( !has_region_id( x, y ) )
									{
										if( has_region_id( x - 1, y ) )
											_region_id[y][x] = _region_id[y][x - 1];
										else
											if( has_region_id( x, y - 1 ) )
												_region_id[ y ][ x ] = _region_id[ y - 1 ][ x ];
											else
												_region_id[ y ][ x ] = ++_last_label; // new label
									}
				}
	}

	std::vector< boost::geometry::model::polygon<point> > connected_component::compute_simplified_contours()
	{
		compute_contours();
		
		std::vector< boost::geometry::model::polygon<point> > simplified_contours( _contours.size() );

		for( size_t i = 0 ; i < _contours.size(); ++i )
		{
			boost::geometry::correct( _contours[i] );
			boost::geometry::simplify( _contours[i], simplified_contours[i].outer(), 0);
		}

		_is_inner = std::vector( simplified_contours.size(), false );
		std::vector< std::vector< boost::geometry::model::polygon<point> > > parts( simplified_contours.size() );
				
		for( size_t i = 0 ; i < simplified_contours.size(); ++i )
			for( size_t j = 0 ; j < simplified_contours.size(); ++j )
			{
				if( i == j )
					continue;

				// if [i] is in [j]
				if( boost::geometry::within( simplified_contours[i].outer(), simplified_contours[j].outer() ) )
				{
					// add [i] as an inner ring of [j]
					simplified_contours[j].inners().push_back( simplified_contours[i].outer() );
					// mark [i] so we know we don't have to consider it
					_is_inner[i] = true;
				}
			}

		for( int i = static_cast<int>( simplified_contours.size() ) - 1 ; i >= 0 ; --i )
			if( _is_inner[i] )
				simplified_contours.erase( simplified_contours.begin() + i );

		for( auto& contour : simplified_contours )
			boost::geometry::correct( contour );

		return simplified_contours;
	}
}
