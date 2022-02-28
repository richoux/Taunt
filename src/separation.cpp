#include "separation.hpp"
#include "region.hpp"

using namespace taunt;

separation::separation( int id, const boost_polygon& polygon )
	: _id(id),
		_polygon( polygon )
{
	point boost_centroid;
	boost::geometry::centroid( _polygon, boost_centroid );

	_centroid = position{ static_cast<position_type>( std::round( boost_centroid.x() ) ), static_cast<position_type>( std::round( boost_centroid.y() ) ) };
	for( const auto& pt : _polygon.outer() )
		_contour.emplace_back( static_cast<position_type>( std::round( pt.x() ) ), static_cast<position_type>( std::round( pt.y() ) ) );
}
