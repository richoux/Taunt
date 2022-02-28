#include "region.hpp"

using namespace taunt;

region::region( int id, const std::vector<const base_location*>& base_locations, const std::vector< const separation*>& separations, const boost_polygon& polygon )
	: _id( id ),
	_base_locations( base_locations ),
	_separations( separations ),
	_polygon( polygon )
{
	point boost_centroid;
	boost::geometry::centroid( _polygon, boost_centroid );
	_centroid = position{ static_cast<position_type>( std::round( boost_centroid.x() ) ), static_cast<position_type>( std::round( boost_centroid.y() ) ) };
}

position region::get_nearest_contour_position( const position& pos )
{
	point p{ static_cast<double>( pos.x ), static_cast<double>( pos.y ) };
	point closest_point;
	double min_distance = std::numeric_limits<double>::max();

	for( const auto& point : _polygon.outer() )
		if( min_distance > boost::geometry::comparable_distance( p, point ) )
		{
			min_distance = boost::geometry::comparable_distance( p, point );
			closest_point = point;
		}

	return {static_cast<position_type>( std::round( closest_point.x() ) ), static_cast<position_type>( std::round( closest_point.y() ) ) };
}

position region::get_entry_position( const separation& sep )
{
	if( std::find_if( _separations.begin(), _separations.end(), [&]( auto s ) {return sep._id == s->_id;} ) == _separations.end() )
		return {-1,-1};

	multipolygon commun;
	boost::geometry::intersection( sep._polygon, _polygon, commun );
	point boost_centroid;
	boost::geometry::centroid( commun, boost_centroid );

	return { static_cast<position_type>( std::round( boost_centroid.x() ) ), static_cast<position_type>( std::round( boost_centroid.y() ) ) };
}